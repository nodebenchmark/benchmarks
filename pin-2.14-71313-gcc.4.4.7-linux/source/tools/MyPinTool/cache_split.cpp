/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2014 Intel Corporation. All rights reserved.
 
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
//
// @ORIGINAL_AUTHOR: Artur Klauser
// @EXTENDED: Rodric Rabbah (rodric@gmail.com) 
//

/*! @file
 *  This file contains an ISA-portable cache simulator
 *  data cache hierarchies
 */


#include "pin.H"

#include <iostream>
#include <fstream>

#include "cache.H"
#include "pin_profile.H"

#include <stdlib.h>
#include <string.h>

std::ofstream outFile;
std::ofstream missSeqFile;
std::ofstream funcHMFile;
PIN_LOCK lock;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<BOOL> KnobICEnable(KNOB_MODE_WRITEONCE,    "pintool",
    "ienable", "0", "enable icache simulation");
KNOB<BOOL> KnobDCEnable(KNOB_MODE_WRITEONCE,    "pintool",
    "denable", "0", "enable dcache simulation");

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,    "pintool",
    "o", "cache.out", "specify icache file name");
KNOB<string> KnobMISSSEQOutputFile(KNOB_MODE_WRITEONCE,    "pintool",
    "f", "missseq.out", "specify output file name for missseq");
KNOB<string> KnobFUNCHMOutputFile(KNOB_MODE_WRITEONCE,    "pintool",
    "m", "funchm.out", "specify output file name for function hit/miss");
KNOB<BOOL>   KnobTrackLoads(KNOB_MODE_WRITEONCE,    "pintool",
    "tl", "0", "track individual loads -- increases profiling time");
KNOB<BOOL>   KnobTrackStores(KNOB_MODE_WRITEONCE,   "pintool",
   "ts", "0", "track individual stores -- increases profiling time");
KNOB<UINT32> KnobThresholdHit(KNOB_MODE_WRITEONCE , "pintool",
   "rh", "100", "only report memops with hit count above threshold");
KNOB<UINT32> KnobThresholdMiss(KNOB_MODE_WRITEONCE, "pintool",
   "rm","100", "only report memops with miss count above threshold");

KNOB<UINT32> KnobDL1CacheSize(KNOB_MODE_WRITEONCE, "pintool",
    "dl1size","32", "cache size in kilobytes");
KNOB<UINT32> KnobDL1LineSize(KNOB_MODE_WRITEONCE, "pintool",
    "dl1line","32", "cache block size in bytes");
KNOB<UINT32> KnobDL1Associativity(KNOB_MODE_WRITEONCE, "pintool",
    "dl1assoc","4", "cache associativity (1 for direct mapped)");

KNOB<UINT32> KnobIL1CacheSize(KNOB_MODE_WRITEONCE, "pintool",
    "il1size","16", "cache size in kilobytes");
KNOB<UINT32> KnobIL1LineSize(KNOB_MODE_WRITEONCE, "pintool",
    "il1line","64", "cache block size in bytes");
KNOB<UINT32> KnobIL1Associativity(KNOB_MODE_WRITEONCE, "pintool",
    "il1assoc","4", "cache associativity (1 for direct mapped)");
KNOB<UINT32> KnobIL1PREFETCH(KNOB_MODE_WRITEONCE, "pintool",
    "il1pf","1", "L1 instruction cache prefetching enabled");
KNOB<INT32> KnobIL1BIPEPSILON(KNOB_MODE_WRITEONCE, "pintool",
    "il1epsilon","-1", "L1 instruction cache BIP EPSILON");
KNOB<UINT32> KnobIL1NativeCacheSize(KNOB_MODE_WRITEONCE, "pintool",
    "il1nsize","16", "native cache size in kilobytes");
KNOB<UINT32> KnobIL1NativePREFETCH(KNOB_MODE_WRITEONCE, "pintool",
    "il1npf","1", "L1 instruction cache prefetching enabled");
KNOB<BOOL> KnobIL1MISSSEQ(KNOB_MODE_WRITEONCE, "pintool",
    "il1missseq","0", "enable collecting L1 instruction cache miss sequence stats");
KNOB<BOOL> KnobIL1FUNCHM(KNOB_MODE_WRITEONCE, "pintool",
    "il1funchm","0", "enable collecting L1 instruction cache function hit/miss");

KNOB<UINT32> KnobL2CacheSize(KNOB_MODE_WRITEONCE, "pintool",
    "l2size","256", "cache size in kilobytes");
KNOB<UINT32> KnobL2LineSize(KNOB_MODE_WRITEONCE, "pintool",
    "l2line","32", "cache block size in bytes");
KNOB<UINT32> KnobL2Associativity(KNOB_MODE_WRITEONCE, "pintool",
    "l2assoc","8", "cache associativity (1 for direct mapped)");

KNOB<BOOL>   KnobAlwaysOn(KNOB_MODE_WRITEONCE, "pintool",
    "always-on", "0", "monitor program regardless of pin_start and pin_end directives");

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr <<
        "This tool represents a cache simulator.\n"
        "\n";

    cerr << KNOB_BASE::StringKnobSummary() << endl; 
    return -1;
}

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

bool alwaysOn;

// wrap configuation constants into their own name space to avoid name clashes
namespace IL1
{
    const UINT32 max_sets = 16 * KILO; // cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = 16; // associativity;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_NO_ALLOCATE;

   	typedef CACHE_BIP(max_sets, max_associativity, allocation) CACHE;
}

IL1::CACHE* il1_v8[128];
IL1::CACHE* il1_native[128];

bool enabled[128];
UINT64 insts_v8[128];
UINT64 insts_native[128];

/* ===================================================================== */

VOID FetchMulti(THREADID threadId, ADDRINT addr, UINT32 size, char *rtn_name)
{
    if (!enabled[threadId])
        return;

	std::string f(rtn_name);
	if(f.compare("invalid_rtn") == 0)
	{
		il1_native[threadId]->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD);
    	insts_native[threadId]++;
	}
	else
	{
		il1_v8[threadId]->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD);
    	insts_v8[threadId]++;
	}
}

/* ===================================================================== */

VOID EnableCache(THREADID threadId)
{
    assert(!enabled[threadId]);
    enabled[threadId] = true;
	cout << "event start" << endl;
}

/* ===================================================================== */

VOID DisableCache(THREADID threadId)
{
    assert(enabled[threadId]);
    enabled[threadId] = false;
	cout << "event stop" << endl;
}

/* ===================================================================== */

UINT64 t_insts = 0;
VOID CountInstructions(THREADID threadId, UINT32 instsInBbl)
{
    if (!enabled[threadId])
        return;

    if(threadId == 0) t_insts += instsInBbl;
}

/* ===================================================================== */

VOID BasicBlock(TRACE trace, void *v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)CountInstructions, IARG_THREAD_ID, IARG_UINT32, BBL_NumIns(bbl), IARG_END);
    }
}

/* ===================================================================== */

VOID Instruction(INS ins, void * v)
{
    // Check for pin_start() and pin_end()
    if (!alwaysOn) {
        if (INS_IsRet(ins)) {
            RTN rtn = INS_Rtn(ins);
            if (RTN_Valid(rtn)) {
                string rtn_name = RTN_Name(rtn);
                if (rtn_name == "pin_start") {
                    cout << "start found" << endl;
                    INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR) EnableCache, IARG_THREAD_ID, IARG_END);
                } else if (rtn_name == "pin_end") {
                    cout << "stop found" << endl;
                    INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR) DisableCache, IARG_THREAD_ID, IARG_END);
                }
            }
        }
    }

    if( KnobICEnable )
	{
        // All instructions go to the instruction cache
        const UINT32 size = INS_Size(ins);

    	RTN rtn = INS_Rtn(ins);
		string rtn_name;
    	if (RTN_Valid(rtn)) rtn_name = RTN_Name(rtn);
		else rtn_name = "invalid_rtn";
		char *p_rtn_name = (char *)malloc((rtn_name.size() + 1) * sizeof(char));
		strcpy(p_rtn_name, rtn_name.c_str());

        INS_InsertCall(
                ins, IPOINT_BEFORE, (AFUNPTR) FetchMulti,
                IARG_THREAD_ID,
                IARG_INST_PTR,
                IARG_UINT32, size,
				IARG_PTR, p_rtn_name,
                IARG_END);
    }
}

/* ===================================================================== */

VOID StartThread(THREADID threadId, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    PIN_GetLock(&lock, threadId+1);

    il1_v8[threadId] = new IL1::CACHE("L1 Instruction Cache",
                                      KnobIL1CacheSize.Value() * KILO,
                                      KnobIL1LineSize.Value(),
                                      KnobIL1Associativity.Value(),
                                      KnobIL1PREFETCH.Value(),
                                      KnobIL1BIPEPSILON.Value());

    il1_native[threadId] = new IL1::CACHE("L1 Instruction Cache",
                                          KnobIL1NativeCacheSize.Value() * KILO,
                                          KnobIL1LineSize.Value(),
                                          KnobIL1Associativity.Value(),
                                          KnobIL1NativePREFETCH.Value(),
                                          KnobIL1BIPEPSILON.Value());

    PIN_ReleaseLock(&lock);
}

/* ===================================================================== */

VOID FiniThread(THREADID threadId, const CONTEXT *ctxt, int code, VOID * v)
{
    PIN_GetLock(&lock, threadId+1);

    //outFile << "PIN:MEMLATENCIES 1.0. 0x0\n";
   	outFile << "Thread " << threadId << " cache statistics\n";

    if( KnobICEnable )
	{
    	il1_v8[threadId]->Instructions(insts_v8[threadId]);

    	outFile <<
    	    "#\n"
    	    "# V8 ICACHE stats\n"
    	    "#\n";

    	outFile << il1_v8[threadId]->StatsLong("# ", CACHE_BASE::CACHE_TYPE_ICACHE);

    	il1_native[threadId]->Instructions(insts_native[threadId]);

    	outFile <<
    	    "#\n"
    	    "# Native ICACHE stats\n"
    	    "#\n";

    	outFile << il1_native[threadId]->StatsLong("# ", CACHE_BASE::CACHE_TYPE_ICACHE);

		if(threadId == 0)
		{
    		outFile << "# Overall-MPKI:\t";
    		outFile << (long double)(il1_native[threadId]->Misses() + il1_v8[threadId]->Misses()) / (long double)(il1_native[threadId]->Instructions() + il1_v8[threadId]->Instructions()) * 1000;
    		outFile << "\n\n";
		}

		if(threadId == 0)
		{
    		if(KnobIL1MISSSEQ.Value()) missSeqFile << il1_v8[threadId]->dumpMissSeq();
    		if(KnobIL1FUNCHM.Value()) funcHMFile << il1_v8[threadId]->funcHMStats();
		}
	}

    PIN_ReleaseLock(&lock);
}

VOID Fini(int code, VOID * v)
{
    outFile.close();
    if(KnobIL1MISSSEQ.Value()) missSeqFile.close();
    if(KnobIL1FUNCHM.Value()) funcHMFile.close();
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    PIN_InitSymbols();

    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }

    alwaysOn = KnobAlwaysOn.Value();
    for (int i = 0; i < 128; ++i) {
        enabled[i] = alwaysOn;
        insts_v8[i] = 0;
        insts_native[i] = 0;
    }

    outFile.open(KnobOutputFile.Value().c_str());
    if(KnobIL1MISSSEQ.Value()) missSeqFile.open(KnobMISSSEQOutputFile.Value().c_str());
    if(KnobIL1FUNCHM.Value()) funcHMFile.open(KnobFUNCHMOutputFile.Value().c_str());
    PIN_InitLock(&lock);
    PIN_AddThreadStartFunction(StartThread, 0);

    INS_AddInstrumentFunction(Instruction, 0);
	TRACE_AddInstrumentFunction(BasicBlock, 0);
    PIN_AddThreadFiniFunction(FiniThread, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    PIN_StartProgram();
    
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
