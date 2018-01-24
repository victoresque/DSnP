/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include "cirGate.h"
#include "cirMgr.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
// Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

static int setMaskInv[32] = {
    ~(1 << 0),  ~(1 << 1),  ~(1 << 2),  ~(1 << 3),  ~(1 << 4),  ~(1 << 5),
    ~(1 << 6),  ~(1 << 7),  ~(1 << 8),  ~(1 << 9),  ~(1 << 10), ~(1 << 11),
    ~(1 << 12), ~(1 << 13), ~(1 << 14), ~(1 << 15), ~(1 << 16), ~(1 << 17),
    ~(1 << 18), ~(1 << 19), ~(1 << 20), ~(1 << 21), ~(1 << 22), ~(1 << 23),
    ~(1 << 24), ~(1 << 25), ~(1 << 26), ~(1 << 27), ~(1 << 28), ~(1 << 29),
    ~(1 << 30), ~(1 << 31)};
static int setMask[32] = {
    1 << 0,  1 << 1,  1 << 2,  1 << 3,  1 << 4,  1 << 5,  1 << 6,  1 << 7,
    1 << 8,  1 << 9,  1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14, 1 << 15,
    1 << 16, 1 << 17, 1 << 18, 1 << 19, 1 << 20, 1 << 21, 1 << 22, 1 << 23,
    1 << 24, 1 << 25, 1 << 26, 1 << 27, 1 << 28, 1 << 29, 1 << 30, 1 << 31};

class SimValPar {
   public:
    typedef int32_t par_type;
    const static par_type mask = 0xffffffff;

    SimValPar(par_type _v = 0) : v(_v) {}
    void set(size_t i, bool b) {
        if (b)
            v |= setMask[i];
        else
            v &= setMaskInv[i];
    }

    bool operator[](size_t i) const { return v & (setMask[i]); }

    size_t operator()() const { return v ^ (v << 1); }
    par_type v;

    void push_back(bool b) {}
    size_t size() { return 32; }
    size_t sizeOfv() { return 1; }
    bool operator==(const SimValPar& rhs) const { return v == rhs.v; }
};

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/

static HashMap<SimValPar, int> FECMapPar;

void CirMgr::randomSim() {
    sweepNoPrompt();
    randomSimParallel();
}

void CirMgr::fileSim(ifstream& patternFile) {
    sweepNoPrompt();
    fileSimParallel(patternFile);
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

void CirMgr::initSim(vector<SimValPar>& val) {
    val[0].v = 0;
    for (int i = 0; i < _LAIGs.size(); i++)
        if (active(toID(_LAIGs[i]))) val[toID(_LAIGs[i])].v = 0;
    for (int i = 0; i < _LOutputs.size(); i++)
        val[toID(_LOutputs[i])].v = 0;
    for (int i = 0; i < _LInputs.size(); i++)
        val[toID(_LInputs[i])].v = 0;
    _FECGroupList.clear();
    _FECGroupList.push_back(list<int>());
    list<int>& list0 = *_FECGroupList.begin();
    list0.push_back(0);
    for (int i = 0; i < _LAIGs.size(); i++)
        if (active(toID(_LAIGs[i]))) list0.push_back(_LAIGs[i]);
}

void writeSimValToGate(vector<SimValPar>& val, CirGate* gate, int id) {
    for (int i = 0; i < val[id].size(); i++)
        gate->_simVal[i] = val[id][i];
}

void CirMgr::fileSimParallel(ifstream& patternFile) {
    vector<SimValPar> val(size() + _LOutputs.size() + 1);
    initSim(val);

    vector<vector<bool>> valPI(_LInputs.size());
    vector<vector<bool>> valPO(_LOutputs.size());

    int T = 0, Tmod32 = 0;
    string in[32];

    bool wrote = false;

        while (patternFile >> in[Tmod32]) {
        ++T, ++Tmod32;
        if (Tmod32 == 32) {
            Tmod32 = 0;
            for (int i = 0; i < _LInputs.size(); i++) {
                int id = toID(_LInputs[i]);
                val[id].v = 0;
                for (int j = 0; j < 32; j++)
                    if (in[j][i] == '1') val[id].v |= setMask[j];
            }
            doSim(val);
            if (_simLog) {
                for (int i = 0; i < 32; i++) {
                    for (int j = 0; j < _LInputs.size(); j++)
                        valPI[j].push_back(val[toID(_LInputs[j])][i]);
                    for (int j = 0; j < _LOutputs.size(); j++)
                        valPO[j].push_back(val[toID(_LOutputs[j])][i]);
                }
            }

            if (T == 32) {
                wrote = true;
                for (int i = 0; i < _LAIGs.size(); i++) {
                    int id = toID(_LAIGs[i]);
                    if (active(id)) writeSimValToGate(val, _gates[id], id);
                }
                for (int i = 0; i < _LInputs.size(); i++) {
                    int id = toID(_LInputs[i]);
                    writeSimValToGate(val, _gates[id], id);
                }
                for (int i = 0; i < _LOutputs.size(); i++) {
                    int id = toID(_LOutputs[i]);
                    writeSimValToGate(val, _gates[id], id);
                }
            }
        }
    }
    if (Tmod32 != 0) {
        for (int i = 0; i < _LInputs.size(); i++) {
            int id = toID(_LInputs[i]);
            val[id].v = 0;
            for (int j = 0; j < 32; j++)
                if (in[j][i] == '1') val[id].v |= setMask[j];
        }
        doSim(val);
        if (_simLog) {
            for (int i = 0; i < 32; i++) {
                for (int j = 0; j < _LInputs.size(); j++)
                    valPI[j].push_back(val[toID(_LInputs[j])][i]);
                for (int j = 0; j < _LOutputs.size(); j++)
                    valPO[j].push_back(val[toID(_LOutputs[j])][i]);
            }
        }
        if(!wrote) {
            for (int i = 0; i < _LAIGs.size(); i++) {
                int id = toID(_LAIGs[i]);
                if (active(id)) writeSimValToGate(val, _gates[id], id);
            }
            for (int i = 0; i < _LInputs.size(); i++) {
                int id = toID(_LInputs[i]);
                writeSimValToGate(val, _gates[id], id);
            }
            for (int i = 0; i < _LOutputs.size(); i++) {
                int id = toID(_LOutputs[i]);
                writeSimValToGate(val, _gates[id], id);
            }
        }
    }

    cout << T << " patterns simulated.\n";
    _FECGroupList.sort();

    if (_simLog) {
        for (int i = 0; i < T; i++) {
            for (int j = 0; j < _LInputs.size(); j++)
                (*_simLog) << valPI[j][i];

            (*_simLog) << " ";
            for (int j = 0; j < _LOutputs.size(); j++)
                (*_simLog) << valPO[j][i];

            (*_simLog) << endl;
        }
    }

    for (auto it = _FECGroupList.begin(); it != _FECGroupList.end(); ++it) {
        auto& FECGroup = *it;
        for (int lid : FECGroup) {
            int id = toID(lid);
            _gates[id]->_FECGroupIt = it;
            _gates[id]->_FECInv = isInv(lid);
        }
    }
}

void CirMgr::simulationParallel(vector<SimValPar>& val, int id) {
    if (visited(id)) return;
    visit(id);

    GateList& LFanIn = LFanInOf(id);
    for (int i = 0; i < LFanIn.size(); i++) {
        int id = toID(LFanIn[i]);
        if (active(id) && !visited(id)) simulationParallel(val, id);
    }

    if (LFanIn.size() == 2) {
        int lid0 = LFanIn[0];
        int lid1 = LFanIn[1];
        SimValPar::par_type mask0 = 0;
        SimValPar::par_type mask1 = 0;

        if (isInv(lid0)) mask0 = SimValPar::mask;
        if (isInv(lid1)) mask1 = SimValPar::mask;
        val[id].v = ((val[toID(lid0)].v ^ mask0) & (val[toID(lid1)].v ^ mask1));
    } else if (LFanIn.size() == 1) {
        int lid0 = LFanIn[0];
        SimValPar::par_type mask0 = 0;
        if (isInv(lid0)) mask0 = SimValPar::mask;
        val[id].v = (val[toID(lid0)].v ^ mask0);
    }
}

void CirMgr::randomSimParallel() {
    vector<SimValPar> val(size() + _LOutputs.size() + 1);
    initSim(val);

    vector<vector<bool>> valPI(_LInputs.size());
    vector<vector<bool>> valPO(_LOutputs.size());

    int T = std::max(2000, (int)(5000 - 10 * sqrt(size())));
    if (_InputNum <= 10) T = std::min(1 << (_InputNum + 2), T);

    int Tmod32 = 0;
    vector<bool> in[32];
    bool wrote = false;
    for (int i = 0; i < 32; i++)
        in[i].resize(_LInputs.size());
    for (int Ti = 0; Ti < T; Ti++) {
        for (int i = 0; i < _LInputs.size(); i++)
            in[Tmod32][i] = rand() & 1;

        ++Tmod32;
        if (Tmod32 == 32) {
            Tmod32 = 0;
            for (int i = 0; i < _LInputs.size(); i++) {
                int id = toID(_LInputs[i]);
                val[id].v = 0;
                for (int j = 0; j < 32; j++)
                    if (in[j][i] == 1) val[id].v |= setMask[j];
            }
            doSim(val);
            if (_simLog) {
                for (int i = 0; i < 32; i++) {
                    for (int j = 0; j < _LInputs.size(); j++)
                        valPI[j].push_back(val[toID(_LInputs[j])][i]);
                    for (int j = 0; j < _LOutputs.size(); j++)
                        valPO[j].push_back(val[toID(_LOutputs[j])][i]);
                }
            }

            if (T == 32) {
                wrote = true;
                for (int i = 0; i < _LAIGs.size(); i++) {
                    int id = toID(_LAIGs[i]);
                    if (active(id)) writeSimValToGate(val, _gates[id], id);
                }
                for (int i = 0; i < _LInputs.size(); i++) {
                    int id = toID(_LInputs[i]);
                    writeSimValToGate(val, _gates[id], id);
                }
                for (int i = 0; i < _LOutputs.size(); i++) {
                    int id = toID(_LOutputs[i]);
                    writeSimValToGate(val, _gates[id], id);
                }
            }
        }
    }
    if (Tmod32 != 0) {
        for (int i = 0; i < _LInputs.size(); i++) {
            int id = toID(_LInputs[i]);
            val[id].v = 0;
            for (int j = 0; j < 32; j++)
                if (in[j][i] == 1) val[id].v |= setMask[j];
        }
        doSim(val);
        if (_simLog) {
            for (int i = 0; i < 32; i++) {
                for (int j = 0; j < _LInputs.size(); j++)
                    valPI[j].push_back(val[toID(_LInputs[j])][i]);
                for (int j = 0; j < _LOutputs.size(); j++)
                    valPO[j].push_back(val[toID(_LOutputs[j])][i]);
            }
        }
        if(!wrote) {
            for (int i = 0; i < _LAIGs.size(); i++) {
                int id = toID(_LAIGs[i]);
                if (active(id)) writeSimValToGate(val, _gates[id], id);
            }
            for (int i = 0; i < _LInputs.size(); i++) {
                int id = toID(_LInputs[i]);
                writeSimValToGate(val, _gates[id], id);
            }
            for (int i = 0; i < _LOutputs.size(); i++) {
                int id = toID(_LOutputs[i]);
                writeSimValToGate(val, _gates[id], id);
            }
        }
    }

    cout << T << " patterns simulated.\n";
    _FECGroupList.sort();

    if (_simLog) {
        for (int i = 0; i < T; i++) {
            for (int j = 0; j < _LInputs.size(); j++)
                (*_simLog) << valPI[j][i];

            (*_simLog) << " ";
            for (int j = 0; j < _LOutputs.size(); j++)
                (*_simLog) << valPO[j][i];

            (*_simLog) << endl;
        }
    }

    for (auto it = _FECGroupList.begin(); it != _FECGroupList.end(); ++it) {
        auto& FECGroup = *it;
        for (int lid : FECGroup) {
            int id = toID(lid);
            _gates[id]->_FECGroupIt = it;
            _gates[id]->_FECInv = isInv(lid);
        }
    }
}

void CirMgr::doSim(vector<SimValPar>& val) {
    resetVisit();
    for (int i = 0; i < _LOutputs.size(); i++)
        simulationParallel(val, toID(_LOutputs[i]));

    auto FECGroupIt = _FECGroupList.begin();
    auto newGroupIt = _FECGroupList.end();
    int sz = _FECGroupList.size();

    HashMap<SimValPar, list<list<int>>::iterator> newGroupMap(32);
    for (int idxOfList = 0; idxOfList < sz; ++idxOfList) {
        list<int>& FECGroup = *FECGroupIt;
        if (FECGroup.size() == 1) {
            FECGroupIt = _FECGroupList.erase(FECGroupIt);
            --sz;
            --idxOfList;
            continue;
        }

        int idxInList = 0;
        newGroupIt = _FECGroupList.end();
        int pivot = 0;

        newGroupMap.clear();
        for (auto it = FECGroup.begin(); it != FECGroup.end();) {
            int lid = *it;
            int id = toID(lid);
            if (idxInList == 0) {
                pivot = val[id].v;
                ++it;
            } else if (!isInv(lid) && val[id].v == pivot) {
                ++it;
            } else if (isInv(lid) && (~val[id].v) == pivot) {
                ++it;
            } else if ((!isInv(lid) && (~val[id].v) == pivot) ||
                       (isInv(lid) && val[id].v == pivot)) {
                *it ^= 1;
                ++it;
            } else {
                bool invVal = false;
                bool found = false;
                if (newGroupMap.count(val[id]) ||
                    (invVal = newGroupMap.count(SimValPar(~val[id].v)))) {
                    found = true;
                    if (!invVal) {
                        auto itNew = newGroupMap[val[id]];
                        (*itNew).push_back(toLID(id, invVal));
                    } else {
                        auto itNew = newGroupMap[SimValPar(~val[id].v)];
                        (*itNew).push_back(toLID(id, invVal));
                    }
                }
                if (!found) {
                    _FECGroupList.push_back(list<int>());
                    _FECGroupList.back().push_back(toLID(id));
                    newGroupIt = _FECGroupList.end();
                    --newGroupIt;
                    newGroupMap[val[id]] = newGroupIt;
                }
                it = FECGroup.erase(it);
            }
            ++idxInList;
        }
        ++FECGroupIt;
    }
}