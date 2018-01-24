/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#include "cirDef.h"

extern CirMgr* cirMgr;

// TODO: Define your own data members and member functions
class CirMgr {
   public:
    CirMgr() {}
    ~CirMgr();

    // Access functions
    // return '0' if "gid" corresponds to an undefined gate.
    CirGate* getGate(unsigned id) const { return _gates[id]; }
    const vector<CirGate*>& getGates() const { return _gates; }

    // Member functions about circuit construction
    bool readCircuit(const string&);
    void reset();

    // Member functions about circuit reporting
    void printSummary() const;
    void printNetlist() const;
    void printPIs() const;
    void printPOs() const;
    void printFloatGates() const;
    void writeAag(ostream&) const;

    int size() const { return _gates.size(); }

   private:
    void setHeader(int M, int I, int L, int O, int A) {
        _maxIndex = M;
        _numInput = I;
        _numLatch = L;
        _numOutput = O;
        _numAIG = A;
        _gates.resize(_maxIndex + _numOutput + 1);
        fill(_gates.begin(), _gates.end(), (CirGate*)NULL);
    }
    void getNotUsed();
    void getNotUsed(int gateId, vector<bool>& vis);
    void printNetlist(int gateId, int& idx, vector<bool>& vis) const;
    void getAIGReachability(int gateId, vector<bool>& vis, vector<int>& aigs) const;

    vector<CirGate*> _gates;
    vector<int> _lidInputs;
    vector<int> _lidOutputs;
    vector<int> _lidAIGs;
    vector<int> _idNotUsed;

    vector<int> _lidOutputsOriginal;

    int _MaxIndex;
    int _InputNum;
    int _LatchNum;
    int _OutputNum;
    int _AIGNum;
};

#endif  // CIR_MGR_H
