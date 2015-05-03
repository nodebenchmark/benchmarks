
/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs 
 *  and could serve as the starting point for developing your first PIN tool
 */

#include <assert.h>

#include <fstream>
#include <iostream>
#include <map>

#include "pin.H"

/* ================================================================== */
// Global variables 
/* ================================================================== */

UINT64 insCount = 0;        //number of dynamically executed instructions
UINT64 bblCount = 0;        //number of dynamically executed basic blocks
UINT64 threadCount = 0;     //total number of threads, including main thread

std::ostream * out = &cerr;

typedef std::map<ADDRINT,UINT64> TargetCount;
struct BranchBehavior
{
    ADDRINT indirectBranchAddr;
    ADDRINT prevTarget;
    std::map<ADDRINT,UINT64> targetCount;

    UINT64 easyTargets;
    UINT64 hardTargets;
};

typedef std::map< ADDRINT,BranchBehavior > BranchTargetMap;
BranchTargetMap eventIndirectTargets;
BranchTargetMap globalIndirectTargets;

bool alwaysOn;
bool enabled[128];

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
    "o", "", "specify file name for MyPinTool output");

KNOB<BOOL>   KnobCount(KNOB_MODE_WRITEONCE,  "pintool",
    "count", "1", "count instructions, basic blocks and threads in the application");

KNOB<BOOL>   KnobAlwaysOn(KNOB_MODE_WRITEONCE, "pintool",
    "always-on", "0", "monitor program regardless of pin_start and pin_end directives");

/* ===================================================================== */
// Utilities
/* ===================================================================== */

/*!
 *  Print out help message.
 */
INT32 Usage()
{
    cerr << "This tool prints out the number of dynamically executed " << endl <<
            "instructions, basic blocks and threads in the application." << endl << endl;

    cerr << KNOB_BASE::StringKnobSummary() << endl;

    return -1;
}

VOID PrintStats(BranchTargetMap &target_map)
{
    // Total the branch targets
    UINT64 target_count = 0;
    UINT64 hard_targets = 0;
    UINT64 easy_targets = 0;

    ADDRINT wildest_branch = 0;
    UINT64 max_targets = 0;
    UINT64 max_total_uses = 0;
    for (BranchTargetMap::iterator i = target_map.begin(),
            ie = target_map.end(); i != ie; ++i) {
        UINT64 unique_targets = i->second.targetCount.size();
        if (unique_targets > max_targets) {
            wildest_branch = i->first;
            max_targets = unique_targets;
            max_total_uses = 0;
            for (std::map<ADDRINT,UINT64>::iterator j = i->second.targetCount.begin(),
                    je = i->second.targetCount.end(); j != je; ++j) {
                max_total_uses += j->second;
            }
            assert(max_total_uses == i->second.easyTargets + i->second.hardTargets);
        }

        target_count += unique_targets;
        easy_targets += i->second.easyTargets;
        hard_targets += i->second.hardTargets;
    }

    *out << target_map.size() << "," << target_count << "," << easy_targets << "," << hard_targets << ",0x" << hex << wildest_branch << "," << dec << max_targets << "," << max_total_uses << endl;
}

/* ===================================================================== */

VOID EnableTracking(THREADID threadId)
{
    assert(!enabled[threadId]);
    enabled[threadId] = true;
}

/* ===================================================================== */

VOID DisableTracking(THREADID threadId)
{
    assert(enabled[threadId]);
    enabled[threadId] = false;

    PrintStats(eventIndirectTargets);
    eventIndirectTargets.clear();
}

/* ===================================================================== */

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */

/*!
 *
 */
VOID CountTargets(THREADID threadId, ADDRINT pc, ADDRINT target)
{
    if (!enabled[threadId])
        return;

    // Check for this branch.
    BranchTargetMap::iterator branch_iter = eventIndirectTargets.find(pc);

    if (branch_iter == eventIndirectTargets.end()) {
        // Make an entry for this branch.
        branch_iter = eventIndirectTargets.insert(std::pair<ADDRINT,BranchBehavior>(pc, BranchBehavior())).first;
        ++branch_iter->second.hardTargets;
    } else {
        if (branch_iter->second.prevTarget == target) {
            ++branch_iter->second.easyTargets;
        } else {
            ++branch_iter->second.hardTargets;
        }
    }

    branch_iter->second.prevTarget = target;

    // Check for this target.
    TargetCount::iterator target_iter = branch_iter->second.targetCount.find(target);

    if (target_iter == branch_iter->second.targetCount.end()) {
        // Make an entry for this target.
        target_iter = branch_iter->second.targetCount.insert(std::pair<ADDRINT,UINT64>(target, 0)).first;
    }

    // Increment the uses of this target.
    ++target_iter->second;




    // Rinse, repeat.

    // Check for this branch.
    branch_iter = globalIndirectTargets.find(pc);

    if (branch_iter == globalIndirectTargets.end()) {
        // Make an entry for this branch.
        branch_iter = globalIndirectTargets.insert(std::pair<ADDRINT,BranchBehavior>(pc, BranchBehavior())).first;
        ++branch_iter->second.hardTargets;
    } else {
        if (branch_iter->second.prevTarget == target) {
            ++branch_iter->second.easyTargets;
        } else {
            ++branch_iter->second.hardTargets;
        }
    }

    branch_iter->second.prevTarget = target;

    // Check for this target.
    target_iter = branch_iter->second.targetCount.find(target);

    if (target_iter == branch_iter->second.targetCount.end()) {
        // Make an entry for this target.
        target_iter = branch_iter->second.targetCount.insert(std::pair<ADDRINT,UINT64>(target, 0)).first;
    }

    // Increment the uses of this target.
    ++target_iter->second;
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

/*!
 *
 */
VOID InstrumentIndirectBranches(INS ins, VOID *v)
{
    // Check for pin_start() and pin_end()
    if (!alwaysOn) {
        if (INS_IsRet(ins)) {
            RTN rtn = INS_Rtn(ins);
            if (RTN_Valid(rtn)) {
                string rtn_name = RTN_Name(rtn);
                if (rtn_name == "pin_start") {
                    INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR) EnableTracking, IARG_THREAD_ID, IARG_END);
                } else if (rtn_name == "pin_end") {
                    INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR) DisableTracking, IARG_THREAD_ID, IARG_END);
                }
            }
        }
    }

    if (INS_IsIndirectBranchOrCall(ins)) {
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)CountTargets, IARG_THREAD_ID, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_END);
    }
}

/*!
 * Print out analysis results.
 * This function is called when the application exits.
 * @param[in]   code            exit code of the application
 * @param[in]   v               value specified by the tool in the 
 *                              PIN_AddFiniFunction function call
 */
VOID ThreadFini(THREADID threadId, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    if (threadId)
        return;

    PrintStats(globalIndirectTargets);
}

/*!
 * The main procedure of the tool.
 * This function is called when the application image is loaded but not yet started.
 * @param[in]   argc            total number of elements in the argv array
 * @param[in]   argv            array of command line arguments, 
 *                              including pin -t <toolname> -- ...
 */
int main(int argc, char *argv[])
{
    PIN_InitSymbols();

    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid 
    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }
    
    alwaysOn = KnobAlwaysOn.Value();
    for (int i = 0; i < 128; ++i) {
        enabled[i] = alwaysOn;
    }

    string fileName = KnobOutputFile.Value();

    if (!fileName.empty()) { out = new std::ofstream(fileName.c_str());}
    *out << "Static Indirect Branches,Static Branch Targets,Dynamic Easy Targets,Dynamic Hard Targets,Wildest Branch,Wildest Branch Unique Targets,Dynamic Targets" << endl;

    // Register function to be called to instrument indirect branches
    INS_AddInstrumentFunction(InstrumentIndirectBranches, 0);

    // Register function to be called when the application exits
    PIN_AddThreadFiniFunction(ThreadFini, 0);

    cerr <<  "===============================================" << endl;
    cerr <<  "This application is instrumented by MyPinTool" << endl;
    if (!KnobOutputFile.Value().empty()) 
    {
        cerr << "See file " << KnobOutputFile.Value() << " for analysis results" << endl;
    }
    cerr <<  "===============================================" << endl;

    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
