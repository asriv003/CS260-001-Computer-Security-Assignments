/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2016 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */

/* ===================================================================== */
/*
  @ORIGINAL_AUTHOR: Abhishek Kumar Srivastava
*/

/* ===================================================================== */
/*! @file
 *  This file contains an ISA-portable PIN tool for tracing instructions
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stack>

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

std::ofstream TraceFile;
std::stack<ADDRINT> shadow;
static bool isOkay = true;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "shadowstack.out", "specify trace file name");
KNOB<BOOL>   KnobPrintArgs(KNOB_MODE_WRITEONCE, "pintool", "a", "0", "print call arguments ");
//KNOB<BOOL>   KnobPrintArgs(KNOB_MODE_WRITEONCE, "pintool", "i", "0", "mark indirect calls ");

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool produces a call trace." << endl << endl;
    cerr << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

string invalid = "invalid_rtn";

/* ===================================================================== */

VOID  do_call(ADDRINT target, ADDRINT ret, THREADID tid)
{
    //const string *t = Target2String(target);
    //TraceFile << "Target: " << target << endl;
    //const string *r = Target2String(ret);
    TraceFile << "Return: " << ret << endl;
    shadow.push(ret);
    //Thread Id can be handled here
}

/* ===================================================================== */

VOID  do_ret(ADDRINT instaddr, ADDRINT target, THREADID tid)
{
    //const string *i = Target2String(instaddr);
    //TraceFile << "Instruction Address: " << instaddr << endl;
    //const string *tr = Target2String(target);
    TraceFile << "Return Adress: " << target << endl;
    if(shadow.top() == target)
    {
        shadow.pop();
    }
    else
    {
        isOkay = false;
        std::cerr << "Error!! Stack Values Does not match!!" << endl;
        TraceFile << "Error!! Stack Values Does not match!!" << endl;
    }

    //Thread Id can be handled here   
}

/* ===================================================================== */


VOID Trace(TRACE trace, VOID *v)
{
    //const BOOL print_args = KnobPrintArgs.Value();
    
        
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        INS inst = BBL_InsTail(bbl);
        
        if(isOkay == true)
        {
            if( INS_IsCall(inst) )
            {
               INS_InsertCall(inst, IPOINT_TAKEN_BRANCH, AFUNPTR(do_call),
                                IARG_BRANCH_TARGET_ADDR, IARG_RETURN_IP, IARG_THREAD_ID, IARG_END);     
            }
            else if( INS_IsRet(inst) )
            {
                INS_InsertCall(inst, IPOINT_BEFORE, AFUNPTR(do_ret), IARG_INST_PTR,
                                IARG_BRANCH_TARGET_ADDR, IARG_THREAD_ID, IARG_END);
            }
        }
    }
}

/* ===================================================================== */

VOID Fini(INT32 code, VOID *v)
{
    TraceFile << "# eof" << endl;
    
    TraceFile.close();
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int  main(int argc, char *argv[])
{
    
    PIN_InitSymbols();

    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }
    

    TraceFile.open(KnobOutputFile.Value().c_str());

    TraceFile << hex;
    TraceFile.setf(ios::showbase);
    
    string trace_header = string("#\n"
                                 "# Call Trace Generated By Pin\n"
                                 "#\n");
    

    TraceFile.write(trace_header.c_str(),trace_header.size());
    
    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    PIN_StartProgram();
    
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
