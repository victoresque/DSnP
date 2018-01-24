/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include "cirGate.h"
#include <stdarg.h>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr* cirMgr;

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void CirGate::reportGate() const {
    string gateReport;
    cout << "==================================================" << endl;
    cout << "= ";

    stringstream ss;
    ss << _id;
    string id_str;
    ss >> id_str;
    ss.clear();
    ss << _lineDefine;
    string line_str;
    ss >> line_str;
    ss.clear();

    gateReport += getTypeStr() + "(" + id_str + ")";
    if (hasSymbol()) gateReport += "\"" + _symbol + "\"";
    gateReport += ", line " + line_str;

    cout << left << setw(47) << gateReport;
    cout << "=" << endl;
    cout << "==================================================" << endl;
}

void CirGate::reportFanin(int level) const {
    assert(level >= 0);
    vector<bool> vis(cirMgr->getGates().size() + 1, false);
    reportFanin(level, 0, vis);
}

void CirGate::reportFanin(int depth, int cur, vector<bool>& vis) const {
    if (!vis[_id]) {
        cout << getTypeStr() << " " << _id << endl;
    } else if (getTypeStr() == "PI" || cur == depth) {
        cout << getTypeStr() << " " << _id << endl;
        return;
    } else {
        cout << getTypeStr() << " " << _id << " (*)" << endl;
        return;
    }
    vis[_id] = true;

    if (cur == depth) return;

    string indent;
    for (int i = 0; i <= cur; i++)
        indent += "  ";

    for (int i = 0; i < _lidFanIn.size(); i++) {
        cout << indent;
        if (_lidFanIn[i] % 2) cout << "!";

        CirGate* gate = cirMgr->getGate(_lidFanIn[i] / 2);
        if (gate)
            gate->reportFanin(depth, cur + 1, vis);
        else
            cout << "UNDEF " << _lidFanIn[i] / 2 << endl;
    }
}

void CirGate::reportFanout(int level) const {
    assert(level >= 0);
    vector<bool> vis(cirMgr->getGates().size() + 1, false);
    reportFanout(level, 0, vis);
}

void CirGate::reportFanout(int depth, int cur, vector<bool>& vis) const {
    if (!vis[_id]) {
        cout << getTypeStr() << " " << _id << endl;
    } else if (getTypeStr() == "PO" || cur == depth) {
        cout << getTypeStr() << " " << _id << endl;
        return;
    } else {
        cout << getTypeStr() << " " << _id << " (*)" << endl;
        return;
    }

    vis[_id] = true;
    if (cur == depth) return;

    string indent;
    for (int i = 0; i <= cur; i++)
        indent += "  ";

    for (int i = 0; i < _lidFanOut.size(); i++) {
        cout << indent;
        if (_lidFanOut[i] % 2) cout << "!";

        CirGate* gate = cirMgr->getGate(_lidFanOut[i] / 2);
        if (gate)
            gate->reportFanout(depth, cur + 1, vis);
        else
            cout << "UNDEF " << _lidFanOut[i] / 2 << endl;
    }
}

void CirAIG::printGate() const {
    cout << "AIG " << _id << " ";
    cout << (cirMgr->getGates()[_lidFanIn[0] / 2] == NULL ? "*" : "")
         << (_lidFanIn[0] % 2 ? "!" : "") << _lidFanIn[0] / 2 << " ";
    cout << (cirMgr->getGates()[_lidFanIn[1] / 2] == NULL ? "*" : "")
         << (_lidFanIn[1] % 2 ? "!" : "") << _lidFanIn[1] / 2;
    if (hasSymbol()) cout << " (" << _symbol << ")";
}
void CirPO::printGate() const {
    cout << "PO  " << _id << " ";
    cout << (cirMgr->getGates()[_lidFanIn[0] / 2] == NULL ? "*" : "")
         << (_lidFanIn[0] % 2 ? "!" : "") << _lidFanIn[0] / 2;
    if (hasSymbol()) cout << " (" << _symbol << ")";
}
void CirPI::printGate() const {
    cout << "PI  " << _id;
    if (hasSymbol()) cout << " (" << _symbol << ")";
}

void CirConst0::printGate() const { cout << "CONST0"; }
