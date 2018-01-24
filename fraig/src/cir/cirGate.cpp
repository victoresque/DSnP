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
#include <string>
#include "cirMgr.h"
#include "util.h"

using namespace std;

// Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
// "CirGate::reportFanout()" for cir cmds. Feel free to define
// your own variables and functions.

extern CirMgr* cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void CirGate::reportGate() const {
    string gateReport;
    cout << "==================================================" << endl;
    cout << "= ";

    gateReport += typeString() + "(" + to_string(_id) + ")";
    if (hasSymbol()) gateReport += "\"" + _symbol + "\"";
    gateReport += ", line " + to_string(_lineNo);

    cout << left << setw(47) << gateReport;
    cout << "=" << endl;

    gateReport = "= FECs:";
    for (int lid : *_FECGroupIt) {
        if (toID(lid) == _id) continue;
        if (_FECInv) {
            gateReport += ((!isInv(lid)) ? " !" : " ") + to_string(toID(lid));
        } else {
            gateReport += ((isInv(lid)) ? " !" : " ") + to_string(toID(lid));
        }
    }
    cout << left << setw(48) << gateReport;
    cout << " =" << endl;

    gateReport = "= Value: ";

    for (int i = 0; i < 32; i++) {
        if (i && i % 4 == 0) gateReport += "_";
        gateReport += to_string(_simVal[i]);
    }
    cout << left << setw(49) << gateReport;
    cout << "=" << endl;
    cout << "==================================================" << endl;
}
void CirGate::reportFanin(int level) const {
    assert(level >= 0);
    vector<bool> vis(cirMgr->size() + 1, false);
    reportFanin(level, 0, vis);
}
bool CirGate::reportFanin(int depth, int cur, vector<bool>& vis) const {
    if (!vis[_id]) {
        cout << typeString() << " " << _id << endl;
    } else if (type() == GATE_PI || cur == depth) {
        cout << typeString() << " " << _id << endl;
        return true;
    } else {
        cout << typeString() << " " << _id << " (*)" << endl;
        return true;
    }

    if (vis[_id]) return true;
    if (cur == depth) return true;

    string indent;
    for (int i = 0; i <= cur; i++)
        indent += "  ";

    bool done = true;
    for (int i = 0; i < _LFanIn.size(); i++) {
        cout << indent;
        if (_LFanIn[i] & 1) cout << "!";

        CirGate* gate = cirMgr->getGateByLID(_LFanIn[i]);
        if (gate)
            done &= gate->reportFanin(depth, cur + 1, vis);
        else
            cout << "UNDEF " << _LFanIn[i] / 2 << endl;
    }
    if (done) vis[_id] = true;
    return done;
}
void CirGate::reportFanout(int level) const {
    assert(level >= 0);
    vector<bool> vis(cirMgr->size() + 1, false);
    reportFanout(level, 0, vis);
}
bool CirGate::reportFanout(int depth, int cur, vector<bool>& vis) const {
    if (!vis[_id]) {
        cout << typeString() << " " << _id << endl;
    } else if (type() == GATE_PO || cur == depth) {
        cout << typeString() << " " << _id << endl;
        return true;
    } else {
        cout << typeString() << " " << _id << " (*)" << endl;
        return true;
    }

    if (vis[_id]) return true;
    if (cur == depth) return true;

    string indent;
    for (int i = 0; i <= cur; i++)
        indent += "  ";

    bool done = true;
    for (int i = 0; i < _LFanOut.size(); i++) {
        cout << indent;
        if (_LFanOut[i] & 1) cout << "!";

        CirGate* gate = cirMgr->getGateByLID(_LFanOut[i]);
        if (gate)
            done &= gate->reportFanout(depth, cur + 1, vis);
        else
            cout << "UNDEF " << _LFanOut[i] / 2 << endl;
    }
    if (done) vis[_id] = true;
    return done;
}
void CirAIG::printGate() const {
    cout << "AIG " << _id << " ";
    cout << (cirMgr->getGateByLID(_LFanIn[0]) == NULL ? "*" : "")
         << (_LFanIn[0] % 2 ? "!" : "") << _LFanIn[0] / 2 << " ";
    cout << (cirMgr->getGateByLID(_LFanIn[1]) == NULL ? "*" : "")
         << (_LFanIn[1] % 2 ? "!" : "") << _LFanIn[1] / 2;
    if (hasSymbol()) cout << " (" << _symbol << ")";
}
void CirPO::printGate() const {
    cout << "PO  " << _id << " ";
    cout << (cirMgr->getGateByLID(_LFanIn[0]) == NULL ? "*" : "")
         << (_LFanIn[0] % 2 ? "!" : "") << _LFanIn[0] / 2;
    if (hasSymbol()) cout << " (" << _symbol << ")";
}
void CirPI::printGate() const {
    cout << "PI  " << _id;
    if (hasSymbol()) cout << " (" << _symbol << ")";
}
void Cir0::printGate() const { cout << "CONST0"; }