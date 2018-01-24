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
#include <cstdio>
#include <string>
#include <list>
#include <vector>

using namespace std;

// Feel free to define your own classes, variables, or functions.
#include "cirDef.h"

extern CirMgr *cirMgr;

inline int toLID(int id) { return id << 1; }
inline int toLID(int id, int inv) { return (id << 1) | (inv & 1); }
inline int toID(int lid) { return lid >> 1; }
inline int isInv(int lid) { return lid & 1; }
inline int inv(int val) { return ~val; }

class SimValPar;

class CirMgr {
   public:
    CirMgr() {}
    ~CirMgr();
    void reset();

    CirGate *getGate(int id) const { return getGateByID(id); }
    CirGate *getGateByID(int id) const { return _gates[id]; }
    CirGate *getGateByLID(int lid) const { return _gates[lid >> 1]; }
    const vector<CirGate *> &getGates() const { return _gates; }

    bool readCircuit(const string &);
    void sweep();
    void optimize();

    void randomSim();
    void fileSim(ifstream &);
    void setSimLog(ofstream *logFile) { _simLog = logFile; }

    void strash();
    void printFEC() const;
    void fraig();

    void printSummary() const;
    void printNetlist() const;
    void printPIs() const;
    void printPOs() const;
    void printFloatGates() const;
    void printFECPairs() const;
    void writeAag(ostream &) const;
    void writeGate(ostream &, CirGate *) const;

    int size() const { return _gates.size(); }

   private:
    void writeGate(int id) const;

    ofstream *_simLog;

    void setHeader(int M, int I, int L, int O, int A);
    void getNotUsed() const;
    void printNetlist(int id, int &idx) const;
    void getAIGReachability(int id, GateList &aigs) const;

    void sweep(int id);
    void sweepNoPrompt();
    void promptRemove(const char* message, int id);

    void optimize(int id);

    void strash(int id);

    void initSim(vector<SimValPar> &val);
    void doSim(vector<SimValPar> &val);
    void randomSimParallel();
    void fileSimParallel(ifstream& patternFile);
    void simulationParallel(vector<SimValPar> &val, int id);

    void replace(int lidNew, int idOld);
    void optReplace(int lidNew, int idOld);
    void strashReplace(int lidNew, int idOld);
    void fraigReplace(int lidNew, int idOld);
    
    void promptReplace(const char* message, int lidNew, int idOld);
    void replaceIn(vector<int> &LFanOut, int lidNew, int idOld);
    void removeOut(vector<int> &LFanIn, int idOld);

    list<list<int>> _FECGroupList;

    bool proveFEC(int lid0, int lid1);
    void buildSolver(SatSolver &ss, int id);

    GatePointerList _gates;
    mutable vector<bool> _active;
    mutable int _activeAIGCount;
    GateList _LInputs;
    GateList _LOutputs;
    GateList _LAIGs;
    mutable GateList _notUsed;

    void getActiveAIGCount() const;
    void getActiveAIGCount(int id) const;

    GateList &LFanInOf(int id);
    GateList &LFanOutOf(int id);

    CirGate *gate(int id) { return _gates[id]; }
    bool isAIG(int id) const;
    bool isPI(int id) const;
    bool isPO(int id) const;
    bool isCONST0(int id) const;

    mutable vector<bool> _cirVis;
    void resetVisit() const { fill(_cirVis.begin(), _cirVis.end(), false); }
    void visit(int id) const { _cirVis[id] = true; }
    bool visited(int id) const { return _cirVis[id]; }

    vector<bool> _replaced;
    void resetReplace() { fill(_replaced.begin(), _replaced.end(), false); }
    void replace(int id) { _replaced[id] = true; }
    bool replaced(int id) const { return _replaced[id]; }
    bool active(int id) const { return _active[id]; }
    bool inactive(int id) const { return !_active[id]; }
    void deactivate(int id);

    int _MaxIndex;
    int _InputNum;
    int _LatchNum;
    int _OutputNum;
    int _AIGNum;
};

#endif  // CIR_MGR_H
