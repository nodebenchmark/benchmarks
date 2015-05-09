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
#include <cassert>
#include <map>
#include "pin.H"

ofstream OutFile;
bool enabled[128];

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
ADDRINT last_end_addr = 0;

// This function is called before every block
VOID docount(THREADID threadId, ADDRINT start_addr, UINT32 size)
{
    if (!enabled[threadId])
        return;

	if(start_addr != last_end_addr + 1)
	{
		if(last_end_addr == 0) OutFile << start_addr << ":";
		else OutFile << last_end_addr << endl << start_addr << ":";
	}
	last_end_addr = start_addr + size - 1;
}
    
// Pin calls this function every time a new basic block is encountered
// It inserts a call to docount
VOID Trace(TRACE trace, VOID *v)
{
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Insert a call to docount before every bbl, passing the number of instructions
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)docount, IARG_THREAD_ID, IARG_ADDRINT, BBL_Address(bbl), IARG_UINT32, BBL_Size(bbl), IARG_END);
    }
}

VOID EnableCache(THREADID threadId)
{
    assert(!enabled[threadId]);
    enabled[threadId] = true;
	OutFile << "event start" << endl;
}

VOID DisableCache(THREADID threadId)
{
    assert(enabled[threadId]);
    enabled[threadId] = false;
	OutFile << "event stop" << endl;
}

std::map<ADDRINT, UINT64> ret_map;

VOID rtn_proc(THREADID threadId, ADDRINT ins_addr)
{
    if (!enabled[threadId])
        return;

	if(ret_map.find(ins_addr) == ret_map.end()) ret_map[ins_addr] = ret_map.size();
	OutFile << ret_map[ins_addr] << endl;
}

VOID Instruction(INS ins, void * v)
{
    // Check for pin_start() and pin_end()
    if (INS_IsRet(ins)) {
        RTN rtn = INS_Rtn(ins);
        if (RTN_Valid(rtn)) {
            string rtn_name = RTN_Name(rtn);
            if (rtn_name == "pin_start") {
				cout << "start found" << endl;
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) EnableCache, IARG_THREAD_ID, IARG_END);
            } else if (rtn_name == "pin_end") {
				cout << "stop found" << endl;
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) DisableCache, IARG_THREAD_ID, IARG_END);
            }
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) rtn_proc, IARG_THREAD_ID, IARG_ADDRINT, INS_Address(ins), IARG_END);
        }
    }
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "bblacs.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
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

int main(int argc, char * argv[])
{
    PIN_InitSymbols();

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    for (int i = 0; i < 128; ++i) {
        enabled[i] = false;
    }

    OutFile.open(KnobOutputFile.Value().c_str());

    INS_AddInstrumentFunction(Instruction, 0);
    //TRACE_AddInstrumentFunction(Trace, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
