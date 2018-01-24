/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <algorithm>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "myHashMap.h"
#include "myHashMap.h"
#include "sat.h"
#include "util.h"

using namespace std;

// Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
// Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

class UnorderedPair {
   public:
    UnorderedPair(int _1, int _2)
        : first(std::min(_1, _2)), second(std::max(_1, _2)) {}
    size_t operator()() const {
        return hash<int>()(first) ^ hash<int>()(second);
    }
    bool operator==(const UnorderedPair& rhs) const {
        return first == rhs.first && second == rhs.second;
    }

   private:
    int first, second;
};

static HashMap<UnorderedPair, int> strashMap;

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed

void CirMgr::strash() {
    resetVisit();
    strashMap.init(_MaxIndex);

    for (int i = 0; i < _LOutputs.size(); i++)
        strash(toID(_LOutputs[i]));
}

vector<Var> gateVar;

void CirMgr::fraig() {
    for (auto& FECGroup : _FECGroupList) {
        if (FECGroup.size() < 2) continue;
        resetReplace();
        list<int>::iterator iti = FECGroup.begin();
        int i, j;
        for (i = 0; iti != FECGroup.end(); ++iti, ++i) {
            int lid0 = *iti;
            if (replaced(i)) continue;
            list<int>::iterator itj = iti;
            int fails = 0;
            for (++itj, j = 0; itj != FECGroup.end(); ++itj, ++j) {
                int lid1 = *itj;
                if (replaced(j)) continue;
                if (proveFEC(lid0, lid1)) {
                    fraigReplace(lid0 ^ (lid1 & 1), toID(lid1));
                    replace(j);
                } else {
                    ++fails;
                }

                if (fails == 8) break;
            }
        }
    }
    sweepNoPrompt();
}

bool CirMgr::proveFEC(int lid0, int lid1) {
    SatSolver solver;
    solver.initialize();
    solver.assumeRelease();

    gateVar.clear();
    gateVar.resize(size());

    resetVisit();
    buildSolver(solver, toID(lid0));
    buildSolver(solver, toID(lid1));

    Var SAT = solver.newVar();
    solver.addXorCNF(SAT, gateVar[toID(lid0)], lid0 & 1, gateVar[toID(lid1)],
                     lid1 & 1);
    solver.assumeProperty(SAT, true);
    return !solver.assumpSolve();
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

void CirMgr::strash(int id) {
    if (visited(id)) return;
    visit(id);
    vector<int>& LFanIn = LFanInOf(id);

    for (int i = 0; i < LFanIn.size(); i++)
        if (_active[toID(LFanIn[i])]) strash(toID(LFanIn[i]));

    if (LFanIn.size() == 2) {
        UnorderedPair p(LFanIn[0], LFanIn[1]);

        if (strashMap.count(p))
            strashReplace(toLID(strashMap[p]), id);
        else
            strashMap[p] = id;
    }
}

void CirMgr::buildSolver(SatSolver& ss, int id) {
    if (visited(id)) return;
    visit(id);

    if (id == 0) {
        gateVar[0] = ss.newVar();
        ss.assumeProperty(gateVar[0], false);
        return;
    }

    assert(gate(id) != nullptr);

    GateList& LFanIn = LFanInOf(id);
    for (int i = 0; i < LFanIn.size(); i++)
        if (active(toID(LFanIn[i]))) buildSolver(ss, toID(LFanIn[i]));

    gateVar[id] = ss.newVar();
    if (LFanIn.size() == 2) {
        int lid0 = LFanIn[0];
        int lid1 = LFanIn[1];
        ss.addAigCNF(gateVar[id], gateVar[toID(lid0)], lid0 & 1,
                     gateVar[toID(lid1)], lid1 & 1);
    }
}

void CirMgr::strashReplace(int lidNew, int idOld) {
    promptReplace("Strashing: ", lidNew, idOld);
    replace(lidNew, idOld);
}

void CirMgr::fraigReplace(int lidNew, int idOld) {
    promptReplace("Fraig: ", lidNew, idOld);
    replace(lidNew, idOld);
}