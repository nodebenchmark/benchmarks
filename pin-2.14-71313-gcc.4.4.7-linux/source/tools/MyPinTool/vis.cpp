/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2015 Intel Corporation. All rights reserved.
 
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
#include <iostream>
#include <fstream>
#include "pin.H"
#include <vector>
#include <map>
#include <assert.h>

using namespace std;

ofstream OutFile;

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static bool enabled = false;
vector<unsigned int> per_event_inscount;

/* ===================================================================== */

VOID EnableCache()
{
    assert(!enabled);
    enabled = true;
	//cout << "event start" << endl;
	OutFile << "event start" << endl;
}

/* ===================================================================== */

VOID DisableCache()
{
    assert(enabled);
    enabled = false;
	//cout << "event stop" << endl;
	OutFile << "event stop" << endl;
}

std::map<ADDRINT, UINT64> ins_map;

VOID ins_dump(THREADID threadId, ADDRINT ins_addr)
{
	if((threadId != 0) || !enabled) return;

	UINT64 hash_ins_addr = ins_addr;

	if(ins_map.find(hash_ins_addr) == ins_map.end()) ins_map[hash_ins_addr] = ins_map.size();
	OutFile << ins_map[hash_ins_addr] << endl;
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    // Check for pin_start() and pin_end()
    if (INS_IsRet(ins)) {
        RTN rtn = INS_Rtn(ins);
        if (RTN_Valid(rtn)) {
            string rtn_name = RTN_Name(rtn);
            if (rtn_name == "pin_start") {
				//cout << "start found" << endl;
                INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR) EnableCache, IARG_END);
            } else if (rtn_name == "pin_end") {
				//cout << "stop found" << endl;
                INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR) DisableCache, IARG_END);
            }
        }
    }

    // Insert a call to docount before every instruction, no arguments are passed
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ins_dump, IARG_THREAD_ID, IARG_ADDRINT, INS_Address(ins), IARG_END);
}

std::map<ADDRINT, UINT64> rtn_map;

VOID rtn_mark(THREADID threadId, ADDRINT rtn_addr)
{
	if((threadId != 0) || !enabled) return;

	if(rtn_map.find(rtn_addr) == rtn_map.end()) rtn_map[rtn_addr] = rtn_map.size();
	OutFile << rtn_map[rtn_addr] << endl;
}

VOID Routine(RTN rtn, VOID *v)
{
    string rtn_name = RTN_Name(rtn);

    RTN_Open(rtn);
    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(rtn_mark), IARG_THREAD_ID, IARG_ADDRINT, INS_Address(RTN_InsHead(rtn)), IARG_END);
    RTN_Close(rtn);
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "rtnaddr.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    PIN_InitSymbols();

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    //RTN_AddInstrumentFunction(Routine, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
