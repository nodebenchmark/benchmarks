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
#include <stdlib.h>
#include <string.h>

using namespace std;

ofstream OutFile;

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static bool enabled[128];
vector<unsigned int> per_event_inscount;
static UINT64 event_id = 0;
map<string, UINT64> funcMap; // mapping from function name to hit and miss counts

/* ===================================================================== */

VOID EnableCache(THREADID threadId)
{
    assert(!enabled[threadId]);
    enabled[threadId] = true;
	cout << "event start" << endl;
	//OutFile << "event start" << endl;
	event_id++;
}

/* ===================================================================== */

VOID DisableCache(THREADID threadId)
{
    assert(enabled[threadId]);
    enabled[threadId] = false;
	cout << "event stop" << endl;
	//OutFile << "event stop" << endl;
}

std::map<ADDRINT, UINT64> ins_map;

VOID process_inst(THREADID threadId, ADDRINT ins_addr, UINT32 size, VOID *rtn_name)
{
    if (!enabled[threadId])
        return;

	//if(event_id >= 120 && event_id <= 127) 
	//{
		char *func = (char *)rtn_name;
		//UINT64 hash_ins_addr = ins_addr;
		//OutFile << hash_ins_addr << " " << size << " " << func << endl;
	//}

	std::string f(func);
	if(funcMap.find(f) == funcMap.end())
		funcMap[f] = 0;
	funcMap[f]++;

	//if(ins_map.find(hash_ins_addr) == ins_map.end()) ins_map[hash_ins_addr] = ins_map.size();
	//OutFile << ins_map[hash_ins_addr] << endl;
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
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) EnableCache, IARG_THREAD_ID, IARG_END);
            } else if (rtn_name == "pin_end") {
				//cout << "stop found" << endl;
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) DisableCache, IARG_THREAD_ID, IARG_END);
            }
        }
    }

    // Insert a call to docount before every instruction, no arguments are passed
    const UINT32 size = INS_Size(ins);
    RTN rtn = INS_Rtn(ins);
	string rtn_name;
    if (RTN_Valid(rtn)) rtn_name = RTN_Name(rtn);
	else rtn_name = "invalid_rtn";
	char *p_rtn_name = (char *)malloc((rtn_name.size() + 1) * sizeof(char));
	strcpy(p_rtn_name, rtn_name.c_str());

   	INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)process_inst, IARG_THREAD_ID, IARG_ADDRINT, INS_Address(ins), IARG_UINT32, size, IARG_PTR, p_rtn_name, IARG_END);
}

std::map<ADDRINT, UINT64> rtn_map;

VOID rtn_mark(THREADID threadId, ADDRINT rtn_addr)
{
    if (!enabled[threadId])
        return;

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
	map<string, UINT64>::iterator it;
	for(it = funcMap.begin();it != funcMap.end();it++)
	{
		OutFile << it->first << " " << it->second << endl;
	}
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

    for (int i = 0; i < 128; ++i) {
        enabled[i] = false;
    }

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    //RTN_AddInstrumentFunction(Routine, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
