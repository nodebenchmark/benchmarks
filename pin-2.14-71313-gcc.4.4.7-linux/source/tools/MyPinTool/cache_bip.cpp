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

std::ofstream outFile;
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
    "il1size","32", "cache size in kilobytes");
KNOB<UINT32> KnobIL1LineSize(KNOB_MODE_WRITEONCE, "pintool",
    "il1line","64", "cache block size in bytes");
KNOB<UINT32> KnobIL1Associativity(KNOB_MODE_WRITEONCE, "pintool",
    "il1assoc","8", "cache associativity (1 for direct mapped)");
KNOB<UINT32> KnobIL1PREFETCH(KNOB_MODE_WRITEONCE, "pintool",
    "il1pf","1", "L1 instruction cache prefetching enabled");
KNOB<INT32> KnobIL1BIPEPSILON(KNOB_MODE_WRITEONCE, "pintool",
    "il1epsilon","-1", "L1 instruction cache BIP EPSILON");

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
namespace DL1
{
    const UINT32 max_sets = 16 * KILO; // cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = 16; // associativity;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

    typedef CACHE_LRU(max_sets, max_associativity, allocation) CACHE;
}

namespace IL1
{
    const UINT32 max_sets = 16 * KILO; // cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = 16; // associativity;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_NO_ALLOCATE;

   	typedef CACHE_BIP(max_sets, max_associativity, allocation) CACHE;
}

namespace L2
{
    const UINT32 max_sets = KILO; // cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = 16; // associativity;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

    typedef CACHE_LRU(max_sets, max_associativity, allocation) CACHE;
}

DL1::CACHE* dl1[128];
IL1::CACHE* il1[128];
//L2::CACHE* l2 = NULL;

typedef enum
{
    COUNTER_MISS = 0,
    COUNTER_HIT = 1,
    COUNTER_NUM
} COUNTER;



typedef  COUNTER_ARRAY<UINT64, COUNTER_NUM> COUNTER_HIT_MISS;


// holds the counters with misses and hits
// conceptually this is an array indexed by instruction address
COMPRESSOR_COUNTER<ADDRINT, UINT32, COUNTER_HIT_MISS> profile;

bool enabled[128];
UINT64 insts[128];

/* ===================================================================== */

VOID LoadMulti(THREADID threadId, ADDRINT addr, UINT32 size, UINT32 instId)
{
    if (!enabled[threadId])
        return;

    // first level D-cache
    const BOOL dl1Hit = dl1[threadId]->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD);
    //if (!dl1Hit) {
    //    l2->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD);
    //}

    const COUNTER counter = dl1Hit ? COUNTER_HIT : COUNTER_MISS;
    profile[instId][counter]++;
}

/* ===================================================================== */

VOID StoreMulti(THREADID threadId, ADDRINT addr, UINT32 size, UINT32 instId)
{
    if (!enabled[threadId])
        return;

    // first level D-cache
    const BOOL dl1Hit = dl1[threadId]->Access(addr, size, CACHE_BASE::ACCESS_TYPE_STORE);
    //if (!dl1Hit) {
    //    l2->Access(addr, size, CACHE_BASE::ACCESS_TYPE_STORE);
    //}

    const COUNTER counter = dl1Hit ? COUNTER_HIT : COUNTER_MISS;
    profile[instId][counter]++;
}

/* ===================================================================== */

VOID LoadSingle(THREADID threadId, ADDRINT addr, UINT32 instId)
{
    if (!enabled[threadId])
        return;

    // @todo we may access several cache lines for 
    // first level D-cache
    const BOOL dl1Hit = dl1[threadId]->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD);
    //if (!dl1Hit) {
    //    l2->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD);
    //}

    const COUNTER counter = dl1Hit ? COUNTER_HIT : COUNTER_MISS;
    profile[instId][counter]++;
}
/* ===================================================================== */

VOID StoreSingle(THREADID threadId, ADDRINT addr, UINT32 instId)
{
    if (!enabled[threadId])
        return;

    // @todo we may access several cache lines for 
    // first level D-cache
    const BOOL dl1Hit = dl1[threadId]->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_STORE);
    //if (!dl1Hit) {
    //    l2->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_STORE);
    //}

    const COUNTER counter = dl1Hit ? COUNTER_HIT : COUNTER_MISS;
    profile[instId][counter]++;
}

/* ===================================================================== */

VOID LoadMultiFast(THREADID threadId, ADDRINT addr, UINT32 size)
{
    if (!enabled[threadId])
        return;

    dl1[threadId]->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD);
    //if (!dl1Hit) {
    //    l2->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD);
    //}
}

/* ===================================================================== */

VOID StoreMultiFast(THREADID threadId, ADDRINT addr, UINT32 size)
{
    if (!enabled[threadId])
        return;

    dl1[threadId]->Access(addr, size, CACHE_BASE::ACCESS_TYPE_STORE);
    //if (!dl1Hit) {
    //    l2->Access(addr, size, CACHE_BASE::ACCESS_TYPE_STORE);
    //}
}

/* ===================================================================== */

VOID LoadSingleFast(THREADID threadId, ADDRINT addr)
{
    if (!enabled[threadId])
        return;

    dl1[threadId]->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD);    
    //if (!dl1Hit) {
    //    l2->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD);
    //}
}

/* ===================================================================== */

VOID StoreSingleFast(THREADID threadId, ADDRINT addr)
{
    if (!enabled[threadId])
        return;

    dl1[threadId]->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_STORE);    
    //if (!dl1Hit) {
    //    l2->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_STORE);
    //}
}

/* ===================================================================== */

VOID FetchSingle(THREADID threadId, ADDRINT addr, UINT32 instId)
{
    if (!enabled[threadId])
        return;

    il1[threadId]->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD);
    //if (!hit) {
    //    l2->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD);
    //}
}

/* ===================================================================== */

VOID FetchMulti(THREADID threadId, ADDRINT addr, UINT32 size, UINT32 instId)
{
    if (!enabled[threadId])
        return;

    il1[threadId]->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD);
    //if (!hit) {
    //    l2->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD);
    //}
}

/* ===================================================================== */

VOID FetchSingleFast(THREADID threadId, ADDRINT addr, UINT32 instId)
{
    if (!enabled[threadId])
        return;

    il1[threadId]->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD);
    //if (!hit) {
    //    l2->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD);
    //}
}

/* ===================================================================== */

VOID FetchMultiFast(THREADID threadId, ADDRINT addr, UINT32 size, UINT32 instId)
{
    if (!enabled[threadId])
        return;

    il1[threadId]->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD);
    //if (!hit) {
    //    l2->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD);
    //}
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

VOID CountInstructions(THREADID threadId, UINT32 instsInBbl)
{
    insts[threadId] += instsInBbl;
}

/* ===================================================================== */

VOID BasicBlock(TRACE trace, void *v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)CountInstructions, IARG_THREAD_ID, IARG_UINT32, BBL_NumIns(bbl), IARG_END);
    }
}

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
        const ADDRINT iaddr = INS_Address(ins);
        const UINT32 instId = profile.Map(iaddr);
        const UINT32 size = INS_Size(ins);
        const BOOL single = (size <= 4);
        if (single) {
            INS_InsertPredicatedCall(
                    ins, IPOINT_BEFORE, (AFUNPTR) FetchSingle,
                    IARG_THREAD_ID,
                    IARG_INST_PTR,
                    IARG_UINT32, instId,
                    IARG_END);
        } else {
            INS_InsertPredicatedCall(
                    ins, IPOINT_BEFORE, (AFUNPTR) FetchMulti,
                    IARG_THREAD_ID,
                    IARG_INST_PTR,
                    IARG_UINT32, size,
                    IARG_UINT32, instId,
                    IARG_END);
        }
    }

    if( KnobDCEnable )
	{
    	if (INS_IsMemoryRead(ins))
    	{
    	    // map sparse INS addresses to dense IDs
    	    const ADDRINT iaddr = INS_Address(ins);
    	    const UINT32 instId = profile.Map(iaddr);

    	    const UINT32 size = INS_MemoryReadSize(ins);
    	    const BOOL   single = (size <= 4);

    	    if( KnobTrackLoads )
    	    {
    	        if( single )
    	        {
    	            INS_InsertPredicatedCall(
    	                ins, IPOINT_BEFORE, (AFUNPTR) LoadSingle,
    	                IARG_THREAD_ID,
    	                IARG_MEMORYREAD_EA,
    	                IARG_UINT32, instId,
    	                IARG_END);
    	        }
    	        else
    	        {
    	            INS_InsertPredicatedCall(
    	                ins, IPOINT_BEFORE,  (AFUNPTR) LoadMulti,
    	                IARG_THREAD_ID,
    	                IARG_MEMORYREAD_EA,
    	                IARG_MEMORYREAD_SIZE,
    	                IARG_UINT32, instId,
    	                IARG_END);
    	        }
    	            
    	    }
    	    else
    	    {
    	        if( single )
    	        {
    	            INS_InsertPredicatedCall(
    	                ins, IPOINT_BEFORE,  (AFUNPTR) LoadSingleFast,
    	                IARG_THREAD_ID,
    	                IARG_MEMORYREAD_EA,
    	                IARG_END);
    	                    
    	        }
    	        else
    	        {
    	            INS_InsertPredicatedCall(
    	                ins, IPOINT_BEFORE,  (AFUNPTR) LoadMultiFast,
    	                IARG_THREAD_ID,
    	                IARG_MEMORYREAD_EA,
    	                IARG_MEMORYREAD_SIZE,
    	                IARG_END);
    	        }
    	    }
    	}
    	    
    	if ( INS_IsMemoryWrite(ins) )
    	{
    	    // map sparse INS addresses to dense IDs
    	    const ADDRINT iaddr = INS_Address(ins);
    	    const UINT32 instId = profile.Map(iaddr);
    	        
    	    const UINT32 size = INS_MemoryWriteSize(ins);

    	    const BOOL   single = (size <= 4);
    	            
    	    if( KnobTrackStores )
    	    {
    	        if( single )
    	        {
    	            INS_InsertPredicatedCall(
    	                ins, IPOINT_BEFORE,  (AFUNPTR) StoreSingle,
    	                IARG_THREAD_ID,
    	                IARG_MEMORYWRITE_EA,
    	                IARG_UINT32, instId,
    	                IARG_END);
    	        }
    	        else
    	        {
    	            INS_InsertPredicatedCall(
    	                ins, IPOINT_BEFORE,  (AFUNPTR) StoreMulti,
    	                IARG_THREAD_ID,
    	                IARG_MEMORYWRITE_EA,
    	                IARG_MEMORYWRITE_SIZE,
    	                IARG_UINT32, instId,
    	                IARG_END);
    	        }
    	            
    	    }
    	    else
    	    {
    	        if( single )
    	        {
    	            INS_InsertPredicatedCall(
    	                ins, IPOINT_BEFORE,  (AFUNPTR) StoreSingleFast,
    	                IARG_THREAD_ID,
    	                IARG_MEMORYWRITE_EA,
    	                IARG_END);
    	                    
    	        }
    	        else
    	        {
    	            INS_InsertPredicatedCall(
    	                ins, IPOINT_BEFORE,  (AFUNPTR) StoreMultiFast,
    	                IARG_THREAD_ID,
    	                IARG_MEMORYWRITE_EA,
    	                IARG_MEMORYWRITE_SIZE,
    	                IARG_END);
    	        }
    	    }
    	}
	}
}

/* ===================================================================== */

VOID StartThread(THREADID threadId, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    PIN_GetLock(&lock, threadId+1);
    dl1[threadId] = new DL1::CACHE("L1 Data Cache", 
                         KnobDL1CacheSize.Value() * KILO,
                         KnobDL1LineSize.Value(),
                         KnobDL1Associativity.Value(),
                         0,
                         -1);

    il1[threadId] = new IL1::CACHE("L1 Instruction Cache",
                         KnobIL1CacheSize.Value() * KILO,
                         KnobIL1LineSize.Value(),
                         KnobIL1Associativity.Value(),
                         KnobIL1PREFETCH.Value(),
                         KnobIL1BIPEPSILON.Value());
    PIN_ReleaseLock(&lock);
}

/* ===================================================================== */

VOID FiniThread(THREADID threadId, const CONTEXT *ctxt, int code, VOID * v)
{
    PIN_GetLock(&lock, threadId+1);

    //outFile << "PIN:MEMLATENCIES 1.0. 0x0\n";
   	outFile << "Thread " << threadId << " cache statistics\n";
    if( KnobDCEnable )
	{
    	dl1[threadId]->Instructions(insts[threadId]);
    	outFile <<
    	    "#\n"
    	    "# DCACHE stats\n"
    	    "#\n";

    	outFile << dl1[threadId]->StatsLong("# ", CACHE_BASE::CACHE_TYPE_DCACHE);

    	if( KnobTrackLoads || KnobTrackStores ) {
    	    outFile <<
    	        "#\n"
    	        "# LOAD stats\n"
    	        "#\n";

    	    outFile << profile.StringLong();
    	}
	}

    if( KnobICEnable )
	{
    	il1[threadId]->Instructions(insts[threadId]);
    	outFile <<
    	    "#\n"
    	    "# ICACHE stats\n"
    	    "#\n";

    	outFile << il1[threadId]->StatsLong("# ", CACHE_BASE::CACHE_TYPE_ICACHE);
	}

    //l2->Instructions(insts[threadId]);
    //outFile <<
    //    "#\n"
    //    "# L2 CACHE stats\n"
    //    "#\n";

    //outFile << l2->StatsLong("# ", CACHE_BASE::CACHE_TYPE_NUM);

    PIN_ReleaseLock(&lock);
}

VOID Fini(int code, VOID * v)
{
    outFile.close();
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
        insts[i] = 0;
    }

    outFile.open(KnobOutputFile.Value().c_str());
    PIN_InitLock(&lock);
    PIN_AddThreadStartFunction(StartThread, 0);

    profile.SetKeyName("iaddr          ");
    profile.SetCounterName("dcache:miss        dcache:hit");

    COUNTER_HIT_MISS threshold;

    threshold[COUNTER_HIT] = KnobThresholdHit.Value();
    threshold[COUNTER_MISS] = KnobThresholdMiss.Value();
    
    profile.SetThreshold( threshold );
    
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
