/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include <cctype>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp

bool
initMemCmd()
{
    if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
            cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
            cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
            cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
        )) {
        cerr << "Registering \"mem\" commands fails... exiting" << endl;
        return false;
    }
    return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string& option)
{
    // check option
    string token;
    if (!CmdExec::lexSingleOption(option, token))
        return CMD_EXEC_ERROR;
    if (token.size()) {
        int b;
        if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj)))) {
            cerr << "Illegal block size (" << token << ")!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
        }
        #ifdef MEM_MGR_H
        mtest.reset(toSizeT(b));
        #else
        mtest.reset();
        #endif // MEM_MGR_H
    }
    else
        mtest.reset();
    return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{  
    os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{  
    cout << setw(15) << left << "MTReset: " 
          << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string& option)
{
    // FIXME: [Error message, MISSING/EXTRA] MTNew
    vector<string> options;
    if(!lexOptions(option, options))
        return CMD_EXEC_ERROR;
    if(options.size()!=1&&options.size()!=3)
        return CmdExec::errorOption(CMD_OPT_MISSING, "");

    bool arrayFlag = false;
    bool numFlag   = false;
    int arraySize  = 1;
    int numObj     = 0;
    int optIdx     = 0;

    while(true){
        if(optIdx>=options.size())
            break;

        if(!numFlag && myStr2Int(options[optIdx], numObj)){
            if(numObj<=0)
                return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[optIdx]);
            numFlag = true;
            ++optIdx;
        }
        else if(!arrayFlag && !myStrNCmp("-Array", options[optIdx], 2)){
            arrayFlag = true;
            ++optIdx;
            if(!myStr2Int(options[optIdx], arraySize))
                return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[optIdx]);
            if(arraySize<=0)
                return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[optIdx]);
            ++optIdx;
        }
        else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[optIdx]);
    }

    //if( mtest.getBlockSize()<sizeof(MemTestObj)*arraySize ) return CMD_EXEC_ERROR;

    if(arrayFlag)
        mtest.newArrs(numObj, arraySize);
    else
        mtest.newObjs(numObj);

    return CMD_EXEC_DONE;
}

void
MTNewCmd::usage(ostream& os) const
{  
    os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{  
    cout << setw(15) << left << "MTNew: " 
          << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
    // FIXME: [Error message, MISSING/EXTRA] MTDelete
    vector<string> options;
    if(!lexOptions(option, options))
        return CMD_EXEC_ERROR;
    if(options.size()!=2&&options.size()!=3)
        return CmdExec::errorOption(CMD_OPT_MISSING, "");
    
    bool arrayFlag = false;
    bool randFlag  = false;
    bool indexFlag = false;
    int objId      = 0;
    int randId     = 0;
    int optIdx     = 0;

    while(true){
        if(optIdx>=options.size())
            break;

        if(!arrayFlag && !myStrNCmp("-Array", options[optIdx], 2)){
            arrayFlag = true;
            ++optIdx;
        }
        else if(!indexFlag && !myStrNCmp("-Index", options[optIdx], 2)){
            indexFlag = true;
            ++optIdx;
            if(!myStr2Int(options[optIdx], objId))
                return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[optIdx]);
            if(objId<0)
                return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[optIdx]);
            ++optIdx;
        }
        else if(!randFlag && !myStrNCmp("-Random", options[optIdx], 2)){
            randFlag = true;
            ++optIdx;
            if(!myStr2Int(options[optIdx], randId))
                return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[optIdx]);
            if(randId<=0)
                return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[optIdx]);
            ++optIdx;
        }
        else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[optIdx]);
    }

    if(randFlag && indexFlag)
        return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[optIdx]);
    if(arrayFlag){
        if(!mtest.getArrListSize()){
            cerr << "Size of array list is 0!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
        }
        if(mtest.getArrListSize()<=objId){
            cerr << "Size of array list (" << mtest.getArrListSize() 
                 << ") is <= " << objId << "!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, "");
        }
    }
    else{
        if(!mtest.getObjListSize()){
            cerr << "Size of object list is 0!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
        }
        if(mtest.getObjListSize()<=objId){
            cerr << "Size of array list (" << mtest.getObjListSize() 
                 << ") is <= " << objId << "!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, "");
        }
    }

    if(!arrayFlag && indexFlag)
        mtest.deleteObj(objId);
    else if(!arrayFlag && randFlag ){
        for(int i=0; i<randId; i++)
            mtest.deleteObj(rnGen((int)mtest.getObjListSize()));
    }
    else if( arrayFlag && indexFlag)
        mtest.deleteArr(objId);
    else if( arrayFlag && randFlag ){
        for(int i=0; i<randId; i++)
            mtest.deleteArr(rnGen((int)mtest.getArrListSize()));
    }
    else
        return CMD_EXEC_ERROR;

    return CMD_EXEC_DONE;
}

void
MTDeleteCmd::usage(ostream& os) const
{  
    os << "Usage: MTDelete <-Index (size_t objId) | "
        << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{  
    cout << setw(15) << left << "MTDelete: " 
          << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string& option)
{
    // check option
    if (option.size())
        return CmdExec::errorOption(CMD_OPT_EXTRA, option);
    mtest.print();

    return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{  
    os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{  
    cout << setw(15) << left << "MTPrint: " 
          << "(memory test) print memory manager info" << endl;
}


