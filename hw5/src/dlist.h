/****************************************************************************
  FileName     [ dlist.h ]
  PackageName  [ util ]
  Synopsis     [ Define doubly linked list package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef DLIST_H
#define DLIST_H

#include <cassert>

template <class T>
class DList;

// DListNode is supposed to be a private class. User don't need to see it.
// Only DList and DList::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class DListNode {
    friend class DList<T>;
    friend class DList<T>::iterator;

    DListNode(const T& d, DListNode<T>* p = 0, DListNode<T>* n = 0)
        : _data(d), _prev(p), _next(n) {}

    T _data;
    DListNode<T>* _prev;
    DListNode<T>* _next;
};

template <class T>
class DList {
   public:
    // TOCHK: DList::DList()
    DList() {
        _head = new DListNode<T>(T());
        _head->_prev = _head->_next = _head;  // _head is a dummy node
        _end = _head;
        _size = 0;
        _isSorted = false;
    }
    // TOCHK: DList::~Dlist()
    ~DList() {
        clear();
        delete _head;
    }

    // DO NOT add any more data member or function for class iterator
    class iterator {
        friend class DList;

       public:
        iterator(DListNode<T>* n = 0) : _node(n) {}
        iterator(const iterator& i) : _node(i._node) {}
        ~iterator() {}  // Should NOT delete _node

        // DONE: DList::iterator::operator*
        const T& operator*() const { return _node->data; }
        T& operator*() { return _node->_data; }
        // DONE: DList::iterator::operator++
        iterator& operator++() {
            _node = _node->_next;
            return *(this);
        }
        // DONE: DList::iterator::operator++(int)
        iterator operator++(int) {
            iterator tmp = *(this);
            ++*this;
            return tmp;
        }
        // DONE: DList::iterator::operator--
        iterator& operator--() {
            _node = _node->_prev;
            return *(this);
        }
        // DONE: DList::iterator::operator--(int)
        iterator operator--(int) {
            iterator tmp = *(this);
            --*this;
            return tmp;
        }
        // DONE: DList::iterator::operator=
        iterator& operator=(const iterator& i) {
            _node = i._node;
            return *(this);
        }
        // DONE: DList::iterator::operator!=
        bool operator!=(const iterator& i) const { return _node != i._node; }
        // DONE: DList::iterator::operator==
        bool operator==(const iterator& i) const { return _node == i._node; }

       private:
        DListNode<T>* _node;
    };

    // DONE: DList::begin()
    iterator begin() const { return iterator(_head); }
    // DONE: DList::end()
    iterator end() const { return iterator(_end); }
    // DONE: DList::empty()
    bool empty() const { return _size == 0; }
    // DONE: DList::size()
    size_t size() const { return _size; }
    // TOCHK: DList::push_back()
    void push_back(const T& x) {
        _isSorted = false;

        if (_size == 0) {
            DListNode<T>* _new = new DListNode<T>(x, NULL, _end);
            _new->_prev = _new;
            _end->_prev = _new;
            _head = _new;
        } else {
            DListNode<T>* _new = new DListNode<T>(x, _end->_prev, _end);
            _new->_prev->_next = _new;
            _end->_prev = _new;
        }
        _size += 1;
    }
    // TOCHK: DList::pop_front()
    void pop_front() {
        if (_size == 0) return;
        if (_size == 1) {
            _head->_prev = _head->_next = _head;
            _end = _head;
        } else {
            _head = _head->_next;
            delete _head->_prev;
            _head->_prev = _head;
        }
        _size -= 1;
    }
    // TOCHK: DList::pop_back()
    void pop_back() {
        if (_size == 0) return;
        if (_size == 1) {
            _head->_prev = _head->_next = _head;
            _end = _head;
        } else {
            DListNode<T>* _back = _end->_prev->_prev;

            delete _end->_prev;
            _back->_next = _end;
            _end->_prev = _back;
        }
        _size -= 1;
    }

    // return false if nothing to erase
    // TOCHK: DList::erase(iterator)
    bool erase(iterator pos) {
        if (_size == 0) return false;
        if (_size == 1) {
            _head->_prev = _head->_next = _head;
            _end = _head;
            _size -= 1;
        } else if (pos._node == _head) {
            pop_front();
        } else {
            DListNode<T>* posNode = pos._node;
            posNode->_prev->_next = posNode->_next;
            posNode->_next->_prev = posNode->_prev;
            delete posNode;
            _size -= 1;
        }

        _isSorted = false;
        return true;
    }
    // TOCHK: DList::erase(const T&)
    bool erase(const T& x) {
        if (_size == 0) return false;

        DListNode<T>* tmp = _head;
        bool found = false;

        while (tmp != tmp->_next) {
            if (tmp->_data == x) {
                found = true;
                break;
            }
            tmp = tmp->_next;
        }

        if (!found)
            return false;
        else
            return erase(iterator(tmp));
    }
    // TOCHK: DList::clear()
    void clear() {
        _size = 0;
        _isSorted = false;
        DListNode<T>* tmp = _head->_next;
        while (tmp && tmp->_next != tmp) {
            DListNode<T>* tmpNext = tmp->_next;
            delete tmp;
            tmp = tmpNext;
        }
        _head->_prev = _head->_next = _head;
        _end = _head;

    }  // delete all nodes except for the dummy node

    // DONE: DList::sort() interface
    void sort() const {
        iterator back = end();
        --back;
        if (!_isSorted) sort(begin(), back, _size);
        _isSorted = true;
    }

   private:
    DListNode<T>* _head;  // = dummy node if list is empty
    DListNode<T>* _end;
    size_t _size;
    mutable bool _isSorted;  // (optionally) to indicate the array is sorted

    // [OPTIONAL TODO] helper functions; called by public member functions
    // TOCHK: DList::sort() implementation
    void sort(iterator b, iterator e, int S) const {
        if (S <= 1) return;
        if (S == 2) {
            if (*e < *b) swap(*e, *b);
            return;
        }
        iterator L = b;
        iterator M = b;
        iterator R = e;
        int SL = S / 2;
        for (int i = 1; i < SL; i++)
            ++M;
        T pivot = *M;

        SL = 0;
        while (L != R) {
            if (pivot < *L) {
                swap(*L, *R);
                --R;
            } else {
                ++L;
                ++SL;
            }
        }

        iterator M1 = L;
        ++M1;
        iterator M2 = L;
        if(M2 != begin()){
            --M2;
            sort(b, M2, SL);
        }
        if(M1 != end()){
            sort(M1, e, S - SL - 1);
        }
    }
};

#endif  // DLIST_H
