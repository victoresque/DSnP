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
#include "cirDef.h"

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate {
    friend class CirMgr;

   public:
    CirGate() {}
    virtual ~CirGate() {}

    // Basic access methods
    virtual string getTypeStr() const = 0;
    unsigned getLineNo() const { return 0; }

    // Printing functions
    void reportGate() const;
    void reportFanin(int level) const;
    void reportFanout(int level) const;
    virtual void printGate() const = 0;

    void setSymbol(string s) { _symbol = s; }
    bool hasSymbol() const { return !_symbol.empty(); }
    string getSymbol() const { return _symbol; }
    void addFanIn(int lid) { _lidFanIn.push_back(lid); }
    void addFanOut(int lid) { _lidFanOut.push_back(lid); }

    void setLine(int l) { _lineDefine = l; }
    void setId(int id) { _id = id; }
    int getId() { return _id; }

   private:
   protected:
    vector<int> _lidFanIn;
    vector<int> _lidFanOut;

    string _symbol;
    int _id;
    int _lineDefine;
    void reportFanin(int depth, int cur, vector<bool>& vis) const;
    void reportFanout(int depth, int cur, vector<bool>& vis) const;
};

class CirAIG : public CirGate {
   public:
    CirAIG() {}
    virtual ~CirAIG() {}
    virtual string getTypeStr() const { return "AIG"; }
    virtual void printGate() const;

   private:
};

class CirPO : public CirGate {
   public:
    CirPO() {}
    virtual ~CirPO() {}
    virtual string getTypeStr() const { return "PO"; }
    virtual void printGate() const;

   private:
};

class CirPI : public CirGate {
   public:
    CirPI() {}
    virtual ~CirPI() {}
    virtual string getTypeStr() const { return "PI"; }
    virtual void printGate() const;

   private:
};

class CirConst0 : public CirGate {
   public:
    CirConst0() {}
    virtual ~CirConst0() {}
    virtual string getTypeStr() const { return "CONST0"; }
    virtual void printGate() const;

   private:
};

#endif  // CIR_GATE_H
