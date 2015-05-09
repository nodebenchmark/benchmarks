
/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs 
 *  and could serve as the starting point for developing your first PIN tool
 */

#include <assert.h>

#include "pin.H"
#include <iostream>
#include <fstream>
#include <map>
#include <set>

/* ================================================================== */
// Global variables 
/* ================================================================== */

std::ostream * out = &cerr;

typedef std::set<ADDRINT> BblBranchSet; // A set of all the branches (should be max 1, but who knows) in a given BBL.
typedef std::map<BblBranchSet*,UINT64> BranchCollection; // Map from each BblBranchSet to the number of its dynamic uses.

BranchCollection *eventBranches;
BranchCollection *applicationBranches;

bool alwaysOn;
bool enabled[128];

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
    "o", "branch_cdf.out", "specify file name for BranchCDF output");

KNOB<BOOL>   KnobAlwaysOn(KNOB_MODE_WRITEONCE, "pintool",
    "always-on", "0", "monitor program regardless of pin_start and pin_end directives");

/* ===================================================================== */
// Utilities
/* ===================================================================== */

/*!
 * Summarize branches.
 */
VOID SummarizeBranches(BranchCollection *branches)
{
    std::map<ADDRINT,UINT64> cdf;
    for (BranchCollection::iterator i = branches->begin(),
            ie = branches->end(); i != ie; ++i) {
        BblBranchSet *branch_set = i->first;
        UINT64 dynamic_count = i->second;

        for (BblBranchSet::iterator j = branch_set->begin(),
                je = branch_set->end(); j != je; ++j) {
            ADDRINT branch_addr = *j;
            std::map<ADDRINT,UINT64>::iterator point = cdf.find(branch_addr);
            if (point == cdf.end()) {
                cdf.insert(std::pair<ADDRINT,UINT64>(branch_addr, dynamic_count));
            } else {
                point->second += dynamic_count;
            }
        }
    }

    // Print out the results
    *out << "Branch Address,Dynamic Instances" << std::endl;
    for (std::map<ADDRINT,UINT64>::iterator i = cdf.begin(),
            ie = cdf.end(); i != ie; ++i) {
        *out << i->first << "," << i->second << std::endl;
    }
    *out << std::endl;
}

VOID EnableTracking(THREADID threadId)
{
    assert(!enabled[threadId]);
    enabled[threadId] = true;
}

VOID DisableTracking(THREADID threadId)
{
    assert(enabled[threadId]);
    enabled[threadId] = false;

    SummarizeBranches(eventBranches);
    eventBranches->clear();
}

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

/*!
 * Create basic block static branch profile.
 */
BblBranchSet *AnalyzeBBL(BBL bbl)
{
    BblBranchSet *branch_set = NULL;
    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {
        // Only interested in conditional branches.
        if (INS_IsBranchOrCall(ins) && INS_HasFallThrough(ins)) {
            if (!branch_set) {
                branch_set = new BblBranchSet;
            }

            ADDRINT branch_addr = INS_Address(ins);
            assert(branch_set->find(branch_addr) == branch_set->end());
            branch_set->insert(branch_addr);
        }

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
    }

    return branch_set;
}

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */

/*!
 * Increase counter of the executed basic blocks and instructions.
 * This function is called for every basic block when it is about to be executed.
 * @param[in]   numInstInBbl    number of instructions in the basic block
 * @note use atomic operations for multi-threaded applications
 */
VOID CountBranches(THREADID threadId, BblBranchSet *branch_set)
{
    if (!enabled[threadId])
        return;

    // Track the branch in this event.
    assert(branch_set);
    BranchCollection::iterator iter = eventBranches->find(branch_set);
    if (iter == eventBranches->end()) {
        iter = eventBranches->insert(std::pair<BblBranchSet*,UINT64>(branch_set, 0)).first;
    }
    ++iter->second;

    // Do the same for global tracking.
    iter = applicationBranches->find(branch_set);
    if (iter == applicationBranches->end()) {
        iter = applicationBranches->insert(std::pair<BblBranchSet*,UINT64>(branch_set, 0)).first;
    }
    ++iter->second;
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

/*!
 * Insert call to the CountBranches() analysis routine before every basic block 
 * of the trace.
 * This function is called every time a new trace is encountered.
 * @param[in]   trace    trace to be instrumented
 * @param[in]   v        value specified by the tool in the TRACE_AddInstrumentFunction
 *                       function call
 */
VOID Trace(TRACE trace, VOID *v)
{
    // Visit every basic block in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        BblBranchSet *branch_set = AnalyzeBBL(bbl);

        if (branch_set) {
            // Insert a call to CountBbl() before every basic block, passing the branch profile
            BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)CountBranches, IARG_THREAD_ID, IARG_PTR, branch_set, IARG_END);
        }
    }
}

/*!
 * Increase counter of threads in the application.
 * This function is called for every thread created by the application when it is
 * about to start running (including the root thread).
 * @param[in]   threadIndex     ID assigned by PIN to the new thread
 * @param[in]   ctxt            initial register state for the new thread
 * @param[in]   flags           thread creation flags (OS specific)
 * @param[in]   v               value specified by the tool in the 
 *                              PIN_AddThreadStartFunction function call
 */
VOID ThreadStart(THREADID threadIndex, CONTEXT *ctxt, INT32 flags, VOID *v)
{
}

/*!
 * Print out analysis results.
 * This function is called when the application exits.
 * @param[in]   code            exit code of the application
 * @param[in]   v               value specified by the tool in the 
 *                              PIN_AddFiniFunction function call
 */
VOID Fini(THREADID threadId, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    if (threadId)
        return;

    SummarizeBranches(applicationBranches);
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

    string fileName = KnobOutputFile.Value();

    if (!fileName.empty()) { out = new std::ofstream(fileName.c_str());}

    alwaysOn = KnobAlwaysOn.Value();
    for (int i = 0; i < 128; ++i) {
        enabled[i] = alwaysOn;
    }

    eventBranches = new BranchCollection;
    applicationBranches = new BranchCollection;

    // Register function to be called to instrument traces
    TRACE_AddInstrumentFunction(Trace, 0);

    // Register function to be called for every thread before it starts running
    PIN_AddThreadStartFunction(ThreadStart, 0);

    // Register function to be called when the application exits
    PIN_AddThreadFiniFunction(Fini, 0);

    cerr <<  "===============================================" << endl;
    cerr <<  "This application is instrumented by BranchCDF" << endl;
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
