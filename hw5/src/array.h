/****************************************************************************
  FileName     [ array.h ]
  PackageName  [ util ]
  Synopsis     [ Define dynamic array package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef ARRAY_H
#define ARRAY_H

#include <cassert>
#include <algorithm>

using namespace std;

// NO need to implement class ArrayNode
//
template <class T>
class Array
{
public:
    Array() : _data(0), _size(0), _capacity(0) {
        _data = new T[_capacity];
    }
    ~Array() { delete []_data; }

    // DO NOT add any more data member or function for class iterator
    class iterator
    {
        friend class Array;

    public:
        iterator(T* n= 0): _node(n) {}
        iterator(const iterator& i): _node(i._node) {}
        ~iterator() {} // Should NOT delete _node

        const T& operator * () const { return *_node; }
        T& operator * () { return *_node; }
        iterator& operator ++ () {
            _node++;
            return *this;
        }
        iterator operator ++ (int) { 
            iterator tmp = *this;
            ++*this;
            return tmp;
        }
        iterator& operator -- () {
            _node--;
            return *this;
        }
        iterator operator -- (int) {
            iterator tmp = *this;
            --*this;
            return tmp;
        }

        iterator operator + (int i) const {
            iterator tmp = *this;
            tmp._node += i;
            return tmp;
        }
        iterator& operator += (int i) {
            _node += i;
            return *this;
        }

        iterator& operator = (const iterator& i) { 
            _node = i._node;
            return *this;
        }

        bool operator != (const iterator& i) const { return _node!=i._node; }
        bool operator == (const iterator& i) const { return _node==i._node; }

    private:
        T*    _node;
    };

    // TODO: implement these functions
    iterator begin() const { return iterator(_data); }
    iterator end() const { return iterator(_data+_size); }
    bool empty() const { return _size==0; }
    size_t size() const { return _size; }

    T& operator [] (size_t i) { return _data[i]; }
    const T& operator [] (size_t i) const { return _data[i]; }

    void push_back(const T& x) {
        if(_size==_capacity)
            reserve(_capacity+1);
        *(_data+_size) = x;
        ++_size;
    }
    void pop_front() {
        --_size;
        *_data = *(_data+_size);
    }
    void pop_back() {
        --_size;
    }

    bool erase(iterator pos) {
        --_size;
        *pos._node = *(_data+_size);
        return true;
    }
    bool erase(const T& x) {
        bool found = false;
        T* t;
        for( t=_data; t!=_data+_size; t++ ){
            if(*t==x){
                found = true;
                break;
            }
        }

        if(!found)  return false; 
        else        return erase(iterator(t));
    }

    void clear() { 
        delete[] _data;
        _data = NULL;
        _size = _capacity = 0;
    }

    // This is done. DO NOT change this one.
    void sort() const { if (!empty()) ::sort(_data, _data+_size); }

    // Nice to have, but not required in this homework...
    void reserve(size_t n) {
        if(n<=_capacity) return;
        while(_capacity<n){
            if(_capacity<1)
                _capacity += 1;
            else
                _capacity *= 2;
        }

        T* _new = new T[_capacity];

        move(_data, _data+_size, _new);
        delete[] _data;
        _data = _new;
    }
    void resize(size_t n) { 
        while(_capacity<n)
            _capacity *= 2;

        reserve(_capacity);
        _size = n;
    }

private:
    T*            _data;
    size_t        _size;       // number of valid elements
    size_t        _capacity;   // max number of elements
    mutable bool  _isSorted;   // (optionally) to indicate the array is sorted

    // [OPTIONAL TODO] Helper functions; called by public member functions
};

#endif // ARRAY_H
