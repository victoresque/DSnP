/****************************************************************************
  FileName     [ myMinHeap.h ]
  PackageName  [ util ]
  Synopsis     [ Define MinHeap ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_MIN_HEAP_H
#define MY_MIN_HEAP_H

#include <algorithm>
#include <vector>

template <class Data>
class MinHeap {
   public:
    MinHeap(size_t s = 0) {
        if (s != 0) _data.reserve(s);
    }
    ~MinHeap() {}

    void clear() { _data.clear(); }

    // For the following member functions,
    // We don't respond for the case vector "_data" is empty!
    const Data& operator[](size_t i) const { return _data[i]; }
    Data& operator[](size_t i) { return _data[i]; }

    size_t size() const { return _data.size(); }

    // TODO
    const Data& min() const { return _data[0]; }
    void insert(const Data& d) {
        _data.push_back(d);

        int n = _data.size() - 1;

        while (n) {
            int par = (n - 1) >> 1;
            if (d < _data[par]) {
                _data[n] = _data[par];
                n = par;
            } else {
                break;
            }
        }
        _data[n] = d;
    }
    void delMin() {
        delData(0);
    }
    void delData(size_t i) {
        if (_data.empty()) return;
        if (i == _data.size() - 1) {
            _data.pop_back();
            return;
        }

        Data d = _data.back();
        _data.pop_back();

        while (true) {
            int par = (i - 1) >> 1;
            int lc = (i << 1) + 1;
            int rc = (i << 1) + 2;
            if (i != 0 && d < _data[par]) {
                _data[i] = _data[par];
                i = par;
            } else if (rc < _data.size()) {  // lc and rc exist
                if (_data[rc] < d && _data[lc] < d) {
                    if (_data[rc] < _data[lc]) {
                        _data[i] = _data[rc];
                        i = rc;
                    } else {
                        _data[i] = _data[lc];
                        i = lc;
                    }
                } else if (_data[lc] < d) {
                    _data[i] = _data[lc];
                    i = lc;
                } else if (_data[rc] < d) {
                    _data[i] = _data[rc];
                    i = rc;
                } else {
                    _data[i] = d;
                    break;
                }

            } else if (lc < _data.size() && _data[lc] < d) {
                _data[i] = _data[lc];
                i = lc;
            } else {
                _data[i] = d;
                break;
            }
        }
    }

   private:
    // DO NOT add or change data members
    vector<Data> _data;
};

#endif  // MY_MIN_HEAP_H
