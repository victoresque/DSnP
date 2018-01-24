/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BST_H
#define BST_H

#include <cassert>
#include <iomanip>
#include <iostream>

using namespace std;

template <class T>
class BSTree;

// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class BSTreeNode {
    friend class BSTree<T>;
    friend class BSTree<T>::iterator;

    BSTreeNode(const T& d, int _rep = 0, BSTreeNode<T>* _l = 0,
               BSTreeNode<T>* _r = 0, BSTreeNode<T>* _p = 0)
        : _data(d), _repeat(_rep), _left(_l), _right(_r), _parent(_p) {}

    T _data;
    int _repeat;
    BSTreeNode<T>* _left;
    BSTreeNode<T>* _right;
    BSTreeNode<T>* _parent;

    void setData(const T& _d, int _r) { _data = _d, _repeat = _r; }
    void setPtr(BSTreeNode<T>* _l, BSTreeNode<T>* _r, BSTreeNode<T>* _p) {
        _left = _l, _right = _r, _parent = _p;
    }
    void setLeft(BSTreeNode<T>* _l) { _left = _l; }
    void setRight(BSTreeNode<T>* _r) { _right = _r; }
    void setParent(BSTreeNode<T>* _p) { _parent = _p; }
    bool isLeftChildOf(BSTreeNode<T>* n) { return n->_left == this; }
    bool isRightChildOf(BSTreeNode<T>* n) { return n->_right == this; }
    bool isEndNode() { return _parent == this; }
    bool hasLeft() { return _left != NULL; }
    bool hasRight() { return _right != NULL; }
    BSTreeNode<T>* minChild() {
        BSTreeNode<T>* node = this;
        while (node->hasLeft())
            node = node->_left;
        return node;
    }
    BSTreeNode<T>* maxChild() {
        BSTreeNode<T>* node = this;
        while (node->hasRight())
            node = node->_right;
        return node;
    }
    BSTreeNode<T>* nextNode() {
        BSTreeNode<T>* node = this;
        if (node->hasRight()) {
            node = node->_right->minChild();
        } else {
            BSTreeNode<T>* tmp;
            tmp = node;
            node = node->_parent;

            while (tmp->isRightChildOf(node)) {
                tmp = node;
                node = node->_parent;
                if (node->isEndNode()) break;
            }
        }
        return node;
    }
    BSTreeNode<T>* prevNode() {
        BSTreeNode<T>* node = this;
        if (node->hasLeft()) {
            node = node->_left->maxChild();
        } else {
            BSTreeNode<T>* tmp;
            tmp = node;
            node = node->_parent;

            while (tmp->isLeftChildOf(node)) {
                tmp = node;
                node = node->_parent;
                if (node->isEndNode()) break;
            }
        }
        return node;
    }
};

template <class T>
class BSTree {
   public:
    BSTree() {
        _end = new BSTreeNode<T>(T());
        _root = new BSTreeNode<T>(T());

        _end->setPtr(_root, _root, _end);
        _root->setParent(_end);

        _size = 0;
        _endIt = iterator(_end, this);
        _beginIt = iterator(_root, this);
    }

    ~BSTree() {
        clear();
        delete _end;
        delete _root;
    }
    class iterator {
        friend class BSTree;

       public:
        iterator(BSTreeNode<T>* n = 0, BSTree<T>* t = 0)
            : _node(n), _tree(t), _repeat(0) {}
        iterator(const iterator& i)
            : _node(i._node), _tree(i._tree), _repeat(i._repeat) {}
        ~iterator() {}
        const T& operator*() const { return _node->data; }
        T& operator*() { return _node->_data; }
        iterator& operator++() {
            ++_repeat;
            if (isLastOfNode()) {
                _node = _node->nextNode();
                _repeat = 0;
            }
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++*this;
            return tmp;
        }

        iterator& operator--() {
            if (*this == _tree->begin()) return *this;
            --_repeat;
            if (isFirstOfNode()) {
                _node = _node->prevNode();
                _repeat = _node->_repeat - 1;
            }
            return *this;
        }

        iterator operator--(int) {
            iterator tmp = *this;
            --*this;
            return tmp;
        }

        iterator& operator=(const iterator& i) {
            _node = i._node;
            _repeat = i._repeat;
            _tree = i._tree;
            return *(this);
        }

        bool operator!=(const iterator& i) const {
            return _node != i._node || _repeat != i._repeat || _tree != i._tree;
        }

        bool operator==(const iterator& i) const {
            return _node == i._node && _repeat == i._repeat && _tree == i._tree;
        }

       private:
        BSTreeNode<T>* _node;
        BSTree<T>* _tree;
        int _repeat;

        bool isLastOfNode() { return _repeat == _node->_repeat; }
        bool isFirstOfNode() { return _repeat == -1; }
    };

    iterator begin() {
        if (_size)
            return _beginIt;
        else
            return _endIt;
    }
    iterator end() { return _endIt; }
    bool empty() const { return _size == 0; }
    size_t size() const { return _size; }

    void insert(const T& x) {
        ++_size;
        if (_size == 1) {
            _root->setData(x, 1);
        } else {
            BSTreeNode<T>* node = _root;
            while (node) {
                if (node->_data == x) {
                    ++node->_repeat;
                    break;
                } else if (x < node->_data) {
                    if (node->hasLeft()) {
                        node = node->_left;
                    } else {
                        node->_left = new BSTreeNode<T>(x, 1, NULL, NULL, node);
                        if (_beginIt._node == node)
                            _beginIt._node = node->_left;
                        break;
                    }
                } else {
                    if (node->hasRight()) {
                        node = node->_right;
                    } else {
                        node->_right =
                            new BSTreeNode<T>(x, 1, NULL, NULL, node);
                        break;
                    }
                }
            }
        }
    }

    void pop_front() { erase(_beginIt); }
    void pop_back() {
        iterator _backIt = _endIt;
        --_backIt;
        erase(_backIt);
    }

    bool erase(iterator pos) {
        BSTreeNode<T>* node = pos._node;

        if (node->_repeat == 1) {
            if (node->hasRight() && node->hasLeft()) {
                BSTreeNode<T>* tmp = node->_right;
                while (tmp->hasLeft())
                    tmp = tmp->_left;

                node->setData(tmp->_data, tmp->_repeat);

                if (tmp->isLeftChildOf(tmp->_parent)) {
                    tmp->_parent->setLeft(tmp->_right);
                    if (tmp->hasRight()) tmp->_right->setParent(tmp->_parent);
                } else if (tmp->isRightChildOf(tmp->_parent)) {
                    tmp->_parent->setRight(tmp->_right);
                    if (tmp->hasRight()) tmp->_right->setParent(tmp->_parent);
                }
                delete tmp;
            } else if (node->hasRight()) {
                if (node->isLeftChildOf(node->_parent)) {
                    node->_parent->setLeft(node->_right);
                    node->_right->setParent(node->_parent);
                    if (node == _root) {
                        _root = node->_right;
                        _end->setLeft(_root);
                        _root->setParent(_end);
                    }
                } else if (node->isRightChildOf(node->_parent)) {
                    node->_parent->setRight(node->_right);
                    node->_right->setParent(node->_parent);
                    if (node == _root) {
                        _root = node->_right;
                        _end->setLeft(_root);
                        _root->setParent(_end);
                    }
                }
                delete node;
            } else if (node->hasLeft()) {
                if (node->isLeftChildOf(node->_parent)) {
                    node->_parent->setLeft(node->_left);
                    node->_left->setParent(node->_parent);
                    if (node == _root) {
                        _root = node->_left;
                        _end->setLeft(_root);
                        _root->setParent(_end);
                    }
                } else if (node->isRightChildOf(node->_parent)) {
                    node->_parent->setRight(node->_left);
                    node->_left->setParent(node->_parent);
                    if (node == _root) {
                        _root = node->_left;
                        _end->setLeft(_root);
                        _root->setParent(_end);
                    }
                }
                delete node;
            } else {
                if (node != _root) {
                    if (node->isLeftChildOf(node->_parent))
                        node->_parent->setLeft(NULL);
                    else if (node->isRightChildOf(node->_parent))
                        node->_parent->setRight(NULL);
                    delete node;
                } else {
                    _root->setLeft(NULL);
                    _root->setRight(NULL);
                }
            }
            updateBegin();
        } else {
            --node->_repeat;
        }
        --_size;
        return true;
    }

    bool erase(const T& x) {
        bool found = false;
        BSTreeNode<T>* node = _root;
        while (node) {
            if (node->_data == x) {
                found = true;
                break;
            } else if (x < node->_data) {
                if (node->hasLeft()) {
                    node = node->_left;
                } else
                    break;
            } else {
                if (node->hasRight()) {
                    node = node->_right;
                } else
                    break;
            }
        }

        if (found)
            return erase(iterator(node, this));
        else
            return false;
    }

    void clear() {
        T dummyT = _root->_data;
        clearTree(_root);

        _root = new BSTreeNode<T>(dummyT);

        _end->setPtr(_root, _root, _end);
        _root->setParent(_end);

        _size = 0;
        _endIt = iterator(_end, this);
        _beginIt = iterator(_root, this);
    }

    void print() {
        /*
        int idxCnt = 0;
        for(iterator it=begin(); it!=end(); ++it ){
            cout << "[" << right << setw(3) << idxCnt << "]"
                 << " = " << left << setw(8) << *it;

            ++idxCnt;
            if(idxCnt%4==0){
                cout << endl;
            }
        }
        */
    }

    void sort() const { return; }

   private:
    BSTreeNode<T>* _root;
    BSTreeNode<T>* _end;
    size_t _size;
    iterator _beginIt;
    iterator _endIt;

    void updateBegin() {
        if (_root->_left == NULL) {
            _beginIt._node = _root;
            return;
        }

        BSTreeNode<T>* tmp = _root;
        while (tmp->_left)
            tmp = tmp->_left;

        _beginIt._node = tmp;
    }

    void clearTree(BSTreeNode<T>* node) {
        if (node->_left) clearTree(node->_left);
        if (node->_right) clearTree(node->_right);
        delete node;
    }
};

#endif  // BST_H
