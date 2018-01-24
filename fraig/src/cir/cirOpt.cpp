/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
// Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed

void CirMgr::promptRemove(const char* message, int id) {
    printf("%sAIG(%d) removed...\n", message, id);
}
void CirMgr::promptReplace(const char* message, int lidNew, int idOld) {
    printf("%s%d merging %s%d...\n", message, toID(lidNew),
           (isInv(lidNew) ? "!" : ""), idOld);
}

void CirMgr::sweep() {
    resetVisit();
    for (int i = 0; i < _LOutputs.size(); i++)
        sweep(toID(_LOutputs[i]));
    for (int i = 0; i < _LAIGs.size(); i++) {
        int id = toID(_LAIGs[i]);
        if (!visited(id) && active(id)) {
            promptRemove("Sweeping: ", id);
            deactivate(id);
            removeOut(LFanInOf(id), id);
        }
    }
}

void CirMgr::sweepNoPrompt() {
    resetVisit();
    for (int i = 0; i < _LOutputs.size(); i++)
        sweep(toID(_LOutputs[i]));
    for (int i = 0; i < _LAIGs.size(); i++) {
        int id = toID(_LAIGs[i]);
        if (!visited(id) && active(id)) {
            deactivate(id);
            removeOut(LFanInOf(id), id);
        }
    }
}
// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void CirMgr::optimize() {
    resetVisit();
    for (int i = 0; i < _LOutputs.size(); i++)
        optimize(toID(_LOutputs[i]));
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
void CirMgr::sweep(int id) {
    if (visited(id)) return;
    visit(id);

    vector<int>& LFanIn = LFanInOf(id);
    for (int i = 0; i < LFanIn.size(); i++) {
        int id = toID(LFanIn[i]);
        if (active(id)) sweep(id);
    }
}

void CirMgr::optimize(int id) {
    if (visited(id)) return;
    visit(id);

    vector<int>& LFanIn = LFanInOf(id);
    for (int i = 0; i < LFanIn.size(); i++) {
        int id = toID(LFanIn[i]);
        if (active(id)) optimize(id);
    }

    if (isAIG(id)) {
        if (LFanIn[0] == 1)
            optReplace(LFanIn[1], id);
        else if (LFanIn[1] == 1)
            optReplace(LFanIn[0], id);
        else if (LFanIn[0] == 0 || LFanIn[1] == 0)
            optReplace(0, id);
        else if (LFanIn[0] == LFanIn[1])
            optReplace(LFanIn[0], id);
        else if (toID(LFanIn[0]) == toID(LFanIn[1]))
            optReplace(0, id);
    }
}

void CirMgr::optReplace(int lidNew, int idOld) {
    promptReplace("Simplifying: ", lidNew, idOld);
    replace(lidNew, idOld);
}

void CirMgr::replace(int lidNew, int idOld) {
    GateList& LFanOutOld = LFanOutOf(idOld);
    int idNew = toID(lidNew);
    GateList& LFanInNew = LFanInOf(idNew);
    GateList& LFanOutNew = LFanOutOf(idNew);
    replaceIn(LFanOutOld, lidNew, idOld);
    removeOut(LFanInNew, idOld);

    if (isInv(lidNew)) {
        for (int i = 0; i < LFanOutOld.size(); i++)
            LFanOutOld[i] ^= 1;
    }
    LFanOutNew.insert(LFanOutNew.end(), LFanOutOld.begin(), LFanOutOld.end());
    deactivate(idOld);
}

void CirMgr::replaceIn(vector<int>& LFanOut, int lidNew, int idOld) {
    for (int i = 0; i < LFanOut.size(); i++) {
        int id = toID(LFanOut[i]);
        for (int j = 0; j < LFanInOf(id).size(); j++) {
            if (toID(LFanInOf(id)[j]) == idOld) {
                if (isInv(LFanInOf(id)[j]))
                    LFanInOf(id)[j] = lidNew ^ 1;
                else
                    LFanInOf(id)[j] = lidNew;
            }
        }
    }
}

void CirMgr::removeOut(vector<int>& LFanIn, int idOld) {
    for (int i = 0; i < LFanIn.size(); i++) {
        int id = toID(LFanIn[i]);
        GateList& LFanOutOfID = LFanOutOf(id);

        int thisIsACounterToOptimizeThisFunctionBecauseItCantBeGreaterThan2 = 0;
        for (int j = 0; j < LFanOutOfID.size(); j++) {
            if (toID(LFanOutOfID[j]) == idOld) {
                LFanOutOfID[j] = LFanOutOfID.back();
                LFanOutOfID.pop_back();
                --j;
                ++thisIsACounterToOptimizeThisFunctionBecauseItCantBeGreaterThan2;
            }
            if (thisIsACounterToOptimizeThisFunctionBecauseItCantBeGreaterThan2 ==
                2)
                break;
        }
    }
}
