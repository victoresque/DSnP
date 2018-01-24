/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate {
   public:
    CirGate(int id) : _simVal(32), _id(id) {}
    virtual ~CirGate() {}

    // Basic access methods
    int id() const { return _id; }
    int lineNo() const { return _lineNo; }
    const string& symbol() const { return _symbol; }
    virtual GateType type() const = 0;
    virtual string typeString() const = 0;

    GateList& LFanIn() { return _LFanIn; }
    GateList& LFanOut() { return _LFanOut; }

    virtual bool isAig() const { return false; }

    // Printing functions
    void reportGate() const;
    void reportFanin(int level) const;
    void reportFanout(int level) const;
    virtual void printGate() const = 0;

    void setLineNo(int line) { _lineNo = line; }
    void setSymbol(const string& s) { _symbol = s; }
    bool hasSymbol() const { return !_symbol.empty(); }

    void addFanIn(int lid) { _LFanIn.push_back(lid); }
    void addFanOut(int lid) { _LFanOut.push_back(lid); }

    list<list<int>>::iterator _FECGroupIt;
    bool _FECInv;
    vector<bool> _simVal;

   private:
   protected:
    GateList _LFanIn;
    GateList _LFanOut;
    string _symbol;
    int _id;
    int _lineNo;

    bool reportFanin(int depth, int cur, vector<bool>& vis) const;
    bool reportFanout(int depth, int cur, vector<bool>& vis) const;
};

#endif  // CIR_GATE_H

class CirAIG : public CirGate {
   public:
    CirAIG(int id) : CirGate(id) {}
    virtual ~CirAIG() {}
    virtual GateType type() const { return GATE_AIG; }
    virtual string typeString() const { return "AIG"; }
    virtual bool isAig() const { return true; }
    virtual void printGate() const;

   private:
};

class CirPO : public CirGate {
   public:
    CirPO(int id) : CirGate(id) {}
    virtual ~CirPO() {}
    virtual GateType type() const { return GATE_PO; }
    virtual string typeString() const { return "PO"; }
    virtual void printGate() const;

   private:
};

class CirPI : public CirGate {
   public:
    CirPI(int id) : CirGate(id) {}
    virtual ~CirPI() {}
    virtual GateType type() const { return GATE_PI; }
    virtual string typeString() const { return "PI"; }
    virtual void printGate() const;

   private:
};

class Cir0 : public CirGate {
   public:
    Cir0(int id) : CirGate(id) {}
    virtual ~Cir0() {}
    virtual GateType type() const { return GATE_CONST0; }
    virtual string typeString() const { return "CONST0"; }
    virtual void printGate() const;

   private:
};
