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
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

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
                 << "\" is redefined, previously defined as "
                 << errGate->getTypeStr() << " in line " << errGate->getLineNo()
                 << "!!" << endl;
            break;
        case REDEF_SYMBOLIC_NAME:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Symbolic name for \""
                 << errMsg << errInt << "\" is redefined!!" << endl;
            break;
        case REDEF_CONST:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Cannot redefine constant (" << errInt << ")!!" << endl;
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
            ss << buf;
            ss >> buf;
            while (ss >> bufInt)
                params.push_back(bufInt);
            assert(params.size() == 5);
            setHeader(params[0], params[1], params[2], params[3], params[4]);
            _gates[0] = new CirConst0();

        } else if (buf[0] == 'c') {
            break;

        } else if (buf[0] == 'i') {
            buf[0] = ' ';
            ss << buf;
            ss >> bufInt >> buf;
            int id = _lidInputs[bufInt] / 2;
            assert(getGate(id) != NULL);
            _gates[id]->setSymbol(buf);
            _gates[id]->setLine(line);

        } else if (buf[0] == 'o') {
            buf[0] = ' ';
            ss << buf;
            ss >> bufInt >> buf;
            int id = _lidOutputs[bufInt] / 2;
            assert(getGate(id) != NULL);
            _gates[id]->setSymbol(buf);

        } else if (isdigit(buf[0])) {
            ss << buf;
            while (ss >> bufInt)
                params.push_back(bufInt);

            assert(params.size() == 1 || params.size() == 3);

            if (params.size() == 1) {
                if (_lidInputs.size() < _numInput) {
                    _lidInputs.push_back(params[0]);
                    int id = params[0] / 2;
                    _gates[id] = new CirPI();
                    _gates[id]->setLine(line);
                    _gates[id]->setId(id);
                } else {
                    _lidOutputsOriginal.push_back(params[0]);
                    _lidOutputs.push_back(params[0]);
                    linePO.push_back(line);
                }

            } else if (params.size() == 3) {
                _lidAIGs.push_back(params[0]);
                int id = params[0] / 2;
                _gates[id] = new CirAIG();
                _gates[id]->addFanIn(params[1]);
                _gates[id]->addFanIn(params[2]);
                _gates[id]->setLine(line);
                _gates[id]->setId(id);
            }

        } else {
        }
    }

    for (int i = 0, j = _maxIndex + 1; i < _numOutput; i++, j++) {
        _gates[j] = new CirPO();
        _gates[j]->addFanIn(_lidOutputs[i]);
        _gates[j]->setSymbol(_gates[_lidOutputs[i] / 2]->getSymbol());
        _gates[_lidOutputs[i] / 2]->setSymbol("");
        _gates[j]->setLine(linePO[i]);
        _gates[j]->setId(j);

        int id = _lidOutputs[i] / 2;
        assert(_gates[id] != NULL);

        _gates[id]->addFanOut(j * 2 + _lidOutputs[i] % 2);
        _lidOutputs[i] = j * 2;
    }
    for (int i = 0; i <= _maxIndex; i++) {
        if (_gates[i] != NULL) {
            const vector<int>& fanIn = _gates[i]->_lidFanIn;
            if (!fanIn.empty()) {
                for (int j = 0; j < fanIn.size(); j++) {
                    if (_gates[fanIn[j] / 2] != NULL)
                        _gates[fanIn[j] / 2]->addFanOut(i * 2 + fanIn[j] % 2);
                }
            }
        }
    }
    aigerFile.close();

    getNotUsed();
    return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void CirMgr::printSummary() const {
    cout << endl;
    cout << "Circuit Statistics" << endl
         << "==================" << endl
         << right << "  PI   " << setw(9) << _lidInputs.size() << endl
         << "  PO   " << setw(9) << _lidOutputs.size() << endl
         << "  AIG  " << setw(9) << _lidAIGs.size() << endl
         << "------------------" << endl
         << "  Total" << setw(9)
         << _lidInputs.size() + _lidOutputs.size() + _lidAIGs.size() << endl;
}

void CirMgr::printNetlist() const {
    cout << endl;

    vector<bool> vis(_gates.size(), false);
    int idx = 0;

    for (int i = 0; i < _lidOutputs.size(); i++)
        printNetlist(_lidOutputs[i] / 2, idx, vis);
    return;
}

void CirMgr::printNetlist(int gateId, int& idx, vector<bool>& vis) const {
    if (vis[gateId]) return;
    vis[gateId] = true;
    CirGate* gate = _gates[gateId];

    for (int i = 0; i < gate->_lidFanIn.size(); i++) {
        CirGate* fanIn = _gates[gate->_lidFanIn[i] / 2];
        if (fanIn != NULL) printNetlist(fanIn->getId(), idx, vis);
    }

    cout << "[" << idx++ << "] ";
    gate->printGate();
    cout << endl;
}

void CirMgr::getNotUsed() {
    for (int i = 1; i <= _maxIndex; i++)
        if (_gates[i] != NULL)
            if (_gates[i]->_lidFanOut.empty()) _idNotUsed.push_back(i);

    return;

    /*
    for (int i = 0; i < _lidInputs.size(); i++) {
        CirGate* gate = _gates[_lidInputs[i] / 2];
        if (gate->_lidFanOut.empty()) _idNotUsed.push_back(_lidInputs[i] / 2);
    }

    vector<bool> vis(_gates.size(), false);
    for (int i = 0; i < _lidOutputs.size(); i++)
        getNotUsed(_lidOutputs[i] / 2, vis);
    for (int i = 0; i < _lidAIGs.size(); i++)
        if (!vis[_lidAIGs[i] / 2]) _idNotUsed.push_back(_lidAIGs[i] / 2);
    */
}
void CirMgr::getNotUsed(int gateId, vector<bool>& vis) {
    if (vis[gateId]) return;
    vis[gateId] = true;
    CirGate* gate = _gates[gateId];
    for (int i = 0; i < gate->_lidFanIn.size(); i++) {
        CirGate* fanIn = _gates[gate->_lidFanIn[i] / 2];
        if (fanIn != NULL) getNotUsed(fanIn->getId(), vis);
    }
}

void CirMgr::printPIs() const {
    cout << "PIs of the circuit:";
    for (int i = 0; i < _lidInputs.size(); i++)
        cout << " " << _lidInputs[i] / 2;
    cout << endl;
}

void CirMgr::printPOs() const {
    cout << "POs of the circuit:";
    for (int i = 0; i < _lidOutputs.size(); i++)
        cout << " " << _lidOutputs[i] / 2;
    cout << endl;
}

void CirMgr::printFloatGates() const {
    bool prompted = false;

    for (int i = 0; i < _lidAIGs.size(); i++) {
        CirGate* gate = _gates[_lidAIGs[i] / 2];
        if (_gates[gate->_lidFanIn[0] / 2] == NULL ||
            _gates[gate->_lidFanIn[1] / 2] == NULL) {
            if (!prompted) {
                cout << "Gates with floating fanin(s):";
                prompted = true;
            }
            cout << " " << _lidAIGs[i] / 2;
        }
    }
    if (prompted) cout << endl;

    if (!_idNotUsed.empty()) {
        cout << "Gates defined but not used  :";
        for (int i = 0; i < _idNotUsed.size(); i++)
            cout << " " << _idNotUsed[i];
        cout << endl;
    }
}

void CirMgr::writeAag(ostream& outfile) const {
    outfile << "aag " << _maxIndex << " " << _numInput << " " << _numLatch
            << " " << _numOutput << " ";

    vector<bool> vis(_gates.size(), false);
    vector<int> idReachableAIGs;
    for (int i = 0; i < _lidOutputs.size(); i++)
        getAIGReachability(_lidOutputs[i] / 2, vis, idReachableAIGs);

    outfile << idReachableAIGs.size() << endl;

    for (int i = 0; i < _lidInputs.size(); i++)
        cout << _lidInputs[i] << endl;
    for (int i = 0; i < _lidOutputsOriginal.size(); i++)
        cout << _lidOutputsOriginal[i] << endl;

    for (int i = 0; i < idReachableAIGs.size(); i++) {
        CirGate* gate = _gates[idReachableAIGs[i]];
        cout << idReachableAIGs[i] * 2 << " ";
        cout << gate->_lidFanIn[0] << " ";
        cout << gate->_lidFanIn[1] << endl;
    }
    cout << "c" << endl;
    cout << "Comment section" << endl;
}

void CirMgr::getAIGReachability(int gateId, vector<bool>& vis,
                                vector<int>& aigs) const {
    if (vis[gateId]) return;
    vis[gateId] = true;
    CirGate* gate = _gates[gateId];

    for (int i = 0; i < gate->_lidFanIn.size(); i++) {
        CirGate* fanIn = _gates[gate->_lidFanIn[i] / 2];
        if (fanIn != NULL) getAIGReachability(fanIn->getId(), vis, aigs);
    }

    if (gate->getTypeStr() == "AIG") aigs.push_back(gateId);
}

void CirMgr::reset() {
    for (int i = 0; i < _gates.size(); i++)
        if (_gates[i] != NULL) delete _gates[i];
    _gates.clear();
    _lidInputs.clear();
    _lidOutputs.clear();
    _lidAIGs.clear();
    _idNotUsed.clear();
    _lidOutputsOriginal.clear();
}
CirMgr::~CirMgr() {
    for (int i = 0; i < _gates.size(); i++)
        if (_gates[i] != NULL) delete _gates[i];
}