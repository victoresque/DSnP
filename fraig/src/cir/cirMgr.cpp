/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include "cirMgr.h"
#include <ctype.h>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "cirGate.h"
#include "util.h"

using namespace std;

// Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
    EXTRA_SPACE,
    MISSING_SPACE,
    ILLEGAL_WSPACE,
    ILLEGAL_NUM,
    ILLEGAL_IDENTIFIER,
    ILLEGAL_SYMBOL_TYPE,
    ILLEGAL_SYMBOL_NAME,
    MISSING_NUM,
    MISSING_IDENTIFIER,
    MISSING_NEWLINE,
    MISSING_DEF,
    CANNOT_INVERTED,
    MAX_LIT_ID,
    REDEF_GATE,
    REDEF_SYMBOLIC_NAME,
    REDEF_CONST,
    NUM_TOO_SMALL,
    NUM_TOO_BIG,

    DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo = 0;   // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate* errGate;

static bool parseError(CirParseError err) {
    switch (err) {
        case EXTRA_SPACE:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Extra space character is detected!!" << endl;
            break;
        case MISSING_SPACE:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Missing space character!!" << endl;
            break;
        case ILLEGAL_WSPACE:  // for non-space white space character
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Illegal white space char(" << errInt << ") is detected!!"
                 << endl;
            break;
        case ILLEGAL_NUM:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Illegal " << errMsg
                 << "!!" << endl;
            break;
        case ILLEGAL_IDENTIFIER:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Illegal identifier \""
                 << errMsg << "\"!!" << endl;
            break;
        case ILLEGAL_SYMBOL_TYPE:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Illegal symbol type (" << errMsg << ")!!" << endl;
            break;
        case ILLEGAL_SYMBOL_NAME:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Symbolic name contains un-printable char(" << errInt
                 << ")!!" << endl;
            break;
        case MISSING_NUM:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Missing " << errMsg << "!!" << endl;
            break;
        case MISSING_IDENTIFIER:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Missing \"" << errMsg
                 << "\"!!" << endl;
            break;
        case MISSING_NEWLINE:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": A new line is expected here!!" << endl;
            break;
        case MISSING_DEF:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Missing " << errMsg
                 << " definition!!" << endl;
            break;
        case CANNOT_INVERTED:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": " << errMsg << " " << errInt << "(" << errInt / 2
                 << ") cannot be inverted!!" << endl;
            break;
        case MAX_LIT_ID:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
                 << endl;
            break;
        case REDEF_GATE:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Literal \"" << errInt
                 << "\" is redefined, previously defined as " << errGate->type()
                 << " in line " << errGate->lineNo() << "!!" << endl;
            break;
        case REDEF_SYMBOLIC_NAME:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Symbolic name for \""
                 << errMsg << errInt << "\" is redefined!!" << endl;
            break;
        case REDEF_CONST:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Cannot redefine const (" << errInt << ")!!" << endl;
            break;
        case NUM_TOO_SMALL:
            cerr << "[ERROR] Line " << lineNo + 1 << ": " << errMsg
                 << " is too small (" << errInt << ")!!" << endl;
            break;
        case NUM_TOO_BIG:
            cerr << "[ERROR] Line " << lineNo + 1 << ": " << errMsg
                 << " is too big (" << errInt << ")!!" << endl;
            break;
        default:
            break;
    }
    return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool CirMgr::readCircuit(const string& fileName) {
    ifstream aigerFile(fileName.c_str());
    if (!aigerFile.is_open()) {
        cout << "Cannot open design \"" << fileName << "\"!!" << endl;
        return false;
    }
    reset();

    string buf;
    int bufInt;
    vector<int> params;
    vector<int> linePO;
    stringstream ss;
    int line = 0;

    while (getline(aigerFile, buf)) {
        ++line;
        params.clear();
        ss.clear();
        if (buf[0] == 'a') {
            ss << buf, ss >> buf;
            while (ss >> bufInt)
                params.push_back(bufInt);
            setHeader(params[0], params[1], params[2], params[3], params[4]);
            _gates[0] = new Cir0(0);
            _active[0] = true;
        } else if (buf[0] == 'c') {
            break;
        } else if (buf[0] == 'i') {
            buf[0] = ' ';
            ss << buf, ss >> bufInt >> buf;
            int id = toID(_LInputs[bufInt]);
            _gates[id]->setSymbol(buf);
        } else if (buf[0] == 'o') {
            buf[0] = ' ';
            ss << buf, ss >> bufInt >> buf;
            int id = toID(_LOutputs[bufInt]);
            _gates[id]->setSymbol(buf);
            _active[id] = true;
        } else if (isdigit(buf[0])) {
            ss << buf;
            while (ss >> bufInt)
                params.push_back(bufInt);
            if (params.size() == 1) {
                if (_LInputs.size() < _InputNum) {
                    _LInputs.push_back(params[0]);
                    int id = toID(params[0]);
                    _gates[id] = new CirPI(id);

                    _active[id] = true;
                    _gates[id]->setLineNo(line);
                } else {
                    _LOutputs.push_back(params[0]);
                    linePO.push_back(line);
                }
            } else if (params.size() == 3) {
                _LAIGs.push_back(params[0]);
                int id = toID(params[0]);
                _gates[id] = new CirAIG(id);
                _active[id] = true;

                _gates[id]->addFanIn(params[1]);
                _gates[id]->addFanIn(params[2]);
                _gates[id]->setLineNo(line);
            } else {
            }
        } else {
        }
    }

    for (int i = 0, id = _MaxIndex + 1; i < _OutputNum; ++i, ++id) {
        _gates[id] = new CirPO(id);
        _active[id] = true;
        int lidOut = _LOutputs[i];
        int idOut = toID(lidOut);

        if (_gates[idOut] != nullptr) {
            _gates[id]->addFanIn(lidOut);
            _gates[id]->setSymbol(_gates[idOut]->symbol());
            if (_gates[idOut]->type() != GATE_PI) _gates[idOut]->setSymbol("");
            _gates[id]->setLineNo(linePO[i]);
            _gates[idOut]->addFanOut(toLID(id, lidOut));
            _LOutputs[i] = toLID(id);
        } else {
            _gates[id]->addFanIn(lidOut);
            _gates[id]->setLineNo(linePO[i]);
            _LOutputs[i] = toLID(id);
        }
    }
    for (int id = 0; id <= _MaxIndex; id++) {
        if (_gates[id] != nullptr) {
            const GateList fanIn = _gates[id]->LFanIn();
            if (!fanIn.empty()) {
                for (int i = 0; i < fanIn.size(); i++) {
                    if (_gates[toID(fanIn[i])] != nullptr)
                        _gates[toID(fanIn[i])]->addFanOut(toLID(id, fanIn[i]));
                }
            }
        }
    }
    aigerFile.close();

    sort(_LAIGs.begin(), _LAIGs.end());
    return true;
}
void CirMgr::printSummary() const {
    getActiveAIGCount();
    cout << endl;
    cout << "Circuit Statistics" << endl
         << "==================" << endl
         << right << "  PI   " << setw(9) << _LInputs.size() << endl
         << "  PO   " << setw(9) << _LOutputs.size() << endl
         << "  AIG  " << setw(9) << _activeAIGCount << endl
         << "------------------" << endl
         << "  Total" << setw(9)
         << _LInputs.size() + _LOutputs.size() + _activeAIGCount << endl;
}
void CirMgr::printNetlist() const {
    cout << endl;
    int idx = 0;

    resetVisit();
    for (int i = 0; i < _LOutputs.size(); i++)
        printNetlist(toID(_LOutputs[i]), idx);
    return;
}
void CirMgr::printNetlist(int id, int& idx) const {
    if (!_active[id]) return;
    if (visited(id)) return;
    visit(id);
    CirGate* gate = _gates[id];

    for (int i = 0; i < gate->LFanIn().size(); i++) {
        CirGate* fanIn = _gates[toID(gate->LFanIn()[i])];
        if (fanIn != nullptr) printNetlist(fanIn->id(), idx);
    }

    cout << "[" << idx++ << "] ";
    gate->printGate();
    cout << endl;
}
void CirMgr::getNotUsed() const {
    for (int id = 1; id <= _MaxIndex; id++)
        if (_gates[id] != nullptr)
            if (_gates[id]->LFanOut().empty()) _notUsed.push_back(id);
    return;
}
void CirMgr::printPIs() const {
    cout << "PIs of the circuit:";
    for (int i = 0; i < _LInputs.size(); i++)
        cout << " " << toID(_LInputs[i]);
    cout << endl;
}
void CirMgr::printPOs() const {
    cout << "POs of the circuit:";
    for (int i = 0; i < _LOutputs.size(); i++)
        cout << " " << toID(_LOutputs[i]);
    cout << endl;
}
void CirMgr::printFloatGates() const {
    getNotUsed();
    bool prompted = false;
    for (int i = 0; i < _LAIGs.size(); i++) {
        int id = toID(_LAIGs[i]);
        if (!_active[id]) continue;
        CirGate* gate = _gates[id];
        if (_gates[toID(gate->LFanIn()[0])] == nullptr ||
            _gates[toID(gate->LFanIn()[1])] == nullptr) {
            if (!prompted) {
                cout << "Gates with floating fanin(s):";
                prompted = true;
            }
            cout << " " << id;
        }
    }
    if (prompted) cout << endl;

    prompted = false;
    if (!_notUsed.empty()) {
        for (int i = 0; i < _notUsed.size(); i++) {
            if (!_active[_notUsed[i]]) continue;
            if (!prompted) {
                cout << "Gates defined but not used  :";
                prompted = true;
            }
            cout << " " << _notUsed[i];
        }
        cout << endl;
    }
}

void CirMgr::printFECPairs() const {
    int i = 0;
    for (auto& feclist : _FECGroupList) {
        if (feclist.size() == 1) continue;
        cout << "[" << i << "]";
        for (auto lid : feclist) {
            cout << " " << ((lid & 1) ? "!" : "") << toID(lid);
        }
        cout << endl;
        ++i;
    }
}
void CirMgr::getAIGReachability(int id, GateList& aigs) const {
    if (visited(id)) return;
    visit(id);
    CirGate* gate = _gates[id];

    for (int i = 0; i < gate->LFanIn().size(); i++) {
        CirGate* fanIn = _gates[toID(gate->LFanIn()[i])];
        if (fanIn != nullptr) getAIGReachability(fanIn->id(), aigs);
    }

    if (isAIG(id)) aigs.push_back(id);
}

void CirMgr::getActiveAIGCount() const {
    _activeAIGCount = 0;

    fill(_active.begin(), _active.end(), false);
    for (int i = 0; i < _LOutputs.size(); i++)
        getActiveAIGCount(toID(_LOutputs[i]));

    _active[0] = true;
    for (int i = 0; i < _LInputs.size(); i++)
        _active[toID(_LInputs[i])] = true;
}
void CirMgr::getActiveAIGCount(int id) const {
    if (_active[id]) return;
    _active[id] = true;
    CirGate* gate = _gates[id];
    GateList& LFanIn = gate->LFanIn();

    for (int i = 0; i < LFanIn.size(); i++) {
        if (_gates[toID(LFanIn[i])] != nullptr)
            getActiveAIGCount(toID(LFanIn[i]));
    }
    if (isAIG(id)) ++_activeAIGCount;
}

void CirMgr::writeAag(ostream& outfile) const {
    outfile << "aag " << _MaxIndex << " " << _InputNum << " " << _LatchNum
            << " " << _OutputNum << " ";

    GateList idReachableAIGs;
    resetVisit();
    for (int i = 0; i < _LOutputs.size(); i++)
        getAIGReachability(toID(_LOutputs[i]), idReachableAIGs);

    outfile << idReachableAIGs.size() << endl;

    for (int i = 0; i < _LInputs.size(); i++)
        outfile << _LInputs[i] << endl;
    for (int i = 0; i < _LOutputs.size(); i++)
        outfile << _gates[toID(_LOutputs[i])]->LFanIn()[0] << endl;

    for (int i = 0; i < idReachableAIGs.size(); i++) {
        CirGate* gate = _gates[idReachableAIGs[i]];
        outfile << toLID(idReachableAIGs[i]) << " ";
        outfile << gate->LFanIn()[0] << " ";
        outfile << gate->LFanIn()[1] << endl;
    }
    for (int i = 0; i < _LInputs.size(); i++) {
        int id = toID(_LInputs[i]);
        CirGate* gate = _gates[id];
        if (gate->hasSymbol())
            outfile << "i" << i << " " << gate->symbol() << endl;
    }
    for (int i = 0; i < _LOutputs.size(); i++) {
        int id = toID(_LOutputs[i]);
        CirGate* gate = _gates[id];
        if (gate->hasSymbol())
            outfile << "o" << i << " " << gate->symbol() << endl;
    }

    outfile << "c" << endl;
    outfile << "Comment comment comment..." << endl;
}

static GateList _LGateInputs;
static GateList _LGateAIGs;
static int _GateMaxIndex;

void CirMgr::writeGate(ostream& outfile, CirGate* g) const {
    _LGateInputs.clear();
    _LGateAIGs.clear();
    int id = g->id();

    resetVisit();
    getActiveAIGCount();
    _GateMaxIndex = 0;
    writeGate(id);

    outfile << "aag " << _GateMaxIndex << " " << _LGateInputs.size() << " 0 1 "
            << _LGateAIGs.size() << endl;

    sort(_LGateInputs.begin(), _LGateInputs.end());
    for (int i = 0; i < _LGateInputs.size(); i++)
        outfile << _LGateInputs[i] << endl;

    cout << toLID(id) << endl;

    for (int i = 0; i < _LGateAIGs.size(); i++) {
        CirGate* gate = _gates[toID(_LGateAIGs[i])];
        outfile << _LGateAIGs[i] << " ";
        outfile << gate->LFanIn()[0] << " " << gate->LFanIn()[1] << endl;
    }
    for (int i = 0; i < _LGateInputs.size(); i++) {
        int id = toID(_LGateInputs[i]);
        CirGate* gate = _gates[id];
        if (gate->hasSymbol())
            outfile << "i" << i << " " << gate->symbol() << endl;
    }
    if (_gates[id]->hasSymbol())
        outfile << "o0 " << _gates[id]->symbol() << endl;
    else
        outfile << "o0 " << id << endl;

    outfile << "c" << endl;
    outfile << "Comment comment comment..." << endl;
}

void CirMgr::writeGate(int id) const {
    if (visited(id)) return;
    visit(id);
    CirGate* gate = _gates[id];
    GateList& LFanIn = gate->LFanIn();

    _GateMaxIndex = std::max(id, _GateMaxIndex);
    if (isAIG(id))
        _LGateAIGs.push_back(toLID(id));
    else if (isPI(id))
        _LGateInputs.push_back(toLID(id));

    for (int i = 0; i < LFanIn.size(); i++) {
        int id = toID(LFanIn[i]);
        if (active(id)) writeGate(id);
    }
}

GateList& CirMgr::LFanInOf(int id) { return _gates[id]->LFanIn(); }
GateList& CirMgr::LFanOutOf(int id) { return _gates[id]->LFanOut(); }
bool CirMgr::isAIG(int id) const { return _gates[id]->type() == GATE_AIG; }
bool CirMgr::isPI(int id) const { return _gates[id]->type() == GATE_PI; }
bool CirMgr::isPO(int id) const { return _gates[id]->type() == GATE_PO; }
bool CirMgr::isCONST0(int id) const {
    return _gates[id]->type() == GATE_CONST0;
}
void CirMgr::deactivate(int id) { _active[id] = false; }

void CirMgr::reset() {
    for (int i = 0; i < _gates.size(); i++)
        if (_gates[i] != nullptr) delete _gates[i];
    _gates.clear();
    _active.clear();
    _LInputs.clear();
    _LOutputs.clear();
    _LAIGs.clear();
    _notUsed.clear();
}
void CirMgr::setHeader(int M, int I, int L, int O, int A) {
    _MaxIndex = M;
    _InputNum = I;
    _LatchNum = L;
    _OutputNum = O;
    _AIGNum = A;
    _gates.resize(_MaxIndex + _OutputNum + 1);
    _active.resize(_gates.size());
    _cirVis.resize(_gates.size());
    _replaced.resize(_gates.size());
    fill(_gates.begin(), _gates.end(), nullptr);
    fill(_active.begin(), _active.end(), false);
}
CirMgr::~CirMgr() {
    for (int i = 0; i < _gates.size(); i++)
        if (_gates[i] != nullptr) delete _gates[i];
}
