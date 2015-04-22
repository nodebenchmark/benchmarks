
/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs 
 *  and could serve as the starting point for developing your first PIN tool
 */

// C includes

// C++ includes
#include <fstream>
#include <iostream>
#include <queue>
#include <map>

// Pin includes
#include "EventQueue.hh"
#include "global.hh"
#include "local.hh"
#include "pin.H"
#include "saturating_counter.hh"
#include "tournament.hh"

/* ================================================================== */
// Global variables 
/* ================================================================== */

UINT64 insCount = 0;        //number of dynamically executed instructions
UINT64 bblCount = 0;        //number of dynamically executed basic blocks
UINT64 threadCount = 0;     //total number of threads, including main thread

std::ostream * out = &cerr;
PIN_LOCK lock;

bool enabled[128];
bool alwaysOn;

typedef uint64_t Time;

Time curTime[128];
Time updateDelay;

//std::map<THREADID, EventQueue> queues;
EventQueue queues[128];
//EventQueue eQueue;
//std::map<THREADID, Predictor *> predictors;
Predictor *predictors[128];
//Predictor *predictor;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
    "o", "", "specify file name for MyPinTool output");

KNOB<string>   KnobPredictorType(KNOB_MODE_WRITEONCE,  "pintool",
    "type", "tournament", "specify type of branch predictor (local, global, tournament)");

KNOB<INT>   KnobGlobalPredictorSize(KNOB_MODE_WRITEONCE,  "pintool",
    "global-size", "4096", "specify number of global branch predictor counters");
KNOB<INT>   KnobLocalPredictorSize(KNOB_MODE_WRITEONCE,  "pintool",
    "local-size", "4096", "specify number of local branch predictor counters");
KNOB<INT>   KnobTournamentPredictorSize(KNOB_MODE_WRITEONCE,  "pintool",
    "tournament-size", "4096", "specify number of tournament branch predictor counters");

KNOB<INT>   KnobLocalPredictorHistories(KNOB_MODE_WRITEONCE,  "pintool",
    "local-histories", "256", "specify number of local branch predictor histories");

KNOB<INT>   KnobUpdateDelay(KNOB_MODE_WRITEONCE,  "pintool",
    "update-delay", "12", "specify number of instructions to wait before updating branch prediction structures");

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

/* ===================================================================== */

VOID EnablePrediction(THREADID threadId)
{
    assert(!enabled[threadId]);
    enabled[threadId] = true;
}

/* ===================================================================== */

VOID DisablePrediction(THREADID threadId)
{
    assert(enabled[threadId]);
    enabled[threadId] = false;
}

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */

/*!
 *
 */
//VOID MakePrediction(ADDRINT addr)
//{
//    bool pred_dir = localPredictor.predict(addr);
//    if (pred_dir) {
//    }
//}

/*!
 *
 */
//uint64_t branches[128];
VOID TakeBranch(ADDRINT addr, THREADID threadId)
{
    if (!enabled[threadId])
        return;

    //++branches[threadId];
    void *state_ptr;
    predictors[threadId]->predict(addr, &state_ptr);

    Event *update_event = predictors[threadId]->createUpdateEvent(predictors[threadId], curTime[threadId] + updateDelay, true, state_ptr);
    queues[threadId].enqueue(update_event);
    predictors[threadId]->updateHistory(true, state_ptr);
}

/*!
 *
 */
VOID FallThroughBranch(ADDRINT addr, THREADID threadId)
{
    if (!enabled[threadId])
        return;

    //++branches[threadId];
    void *state_ptr;
    predictors[threadId]->predict(addr, &state_ptr);

    Event *update_event = predictors[threadId]->createUpdateEvent(predictors[threadId], curTime[threadId] + updateDelay, false, state_ptr);
    queues[threadId].enqueue(update_event);
    predictors[threadId]->updateHistory(false, state_ptr);
}

VOID AdvanceTime(THREADID threadId)
{
    queues[threadId].processUntil(curTime[threadId]);
    ++curTime[threadId];
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

/*!
 * Insert call to make branch prediction.
 */
VOID Branch(INS ins, void *v)
{
    // Check for pin_start() and pin_end()
    if (!alwaysOn) {
        if (INS_IsRet(ins)) {
            RTN rtn = INS_Rtn(ins);
            if (RTN_Valid(rtn)) {
                string rtn_name = RTN_Name(rtn);
                if (rtn_name == "pin_start") {
                    INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR) EnablePrediction, IARG_THREAD_ID, IARG_END);
                } else if (rtn_name == "pin_end") {
                    INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR) DisablePrediction, IARG_THREAD_ID, IARG_END);
                }
            }
        }
    }

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)AdvanceTime, IARG_THREAD_ID, IARG_END);
    if (INS_IsBranchOrCall(ins) && INS_HasFallThrough(ins)) {
        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)TakeBranch, IARG_ADDRINT, INS_Address(ins), IARG_THREAD_ID, IARG_END);
        INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)FallThroughBranch, IARG_ADDRINT, INS_Address(ins), IARG_THREAD_ID, IARG_END);
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
    threadCount++;
}

VOID threadCreated(THREADID threadIndex, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    PIN_GetLock(&lock, threadIndex+1);

    if (KnobPredictorType.Value() == "local") {
        predictors[threadIndex] = new
            LocalPredictor(KnobLocalPredictorSize.Value(),
                    KnobLocalPredictorHistories.Value());
    } else if (KnobPredictorType.Value() == "global") {
        predictors[threadIndex] = new
            GlobalPredictor(KnobGlobalPredictorSize.Value());
    } else if (KnobPredictorType.Value() == "tournament") {
        predictors[threadIndex] = new
            TournamentPredictor(KnobLocalPredictorSize.Value(),
                    KnobLocalPredictorHistories.Value(),
                    KnobGlobalPredictorSize.Value(),
                    KnobTournamentPredictorSize.Value());
    } else {
        cerr << "Error: Unknown predictor type " << KnobPredictorType.Value() << endl;
        cerr << "Valid options: 'local', 'global', 'tournament'" << endl;
        exit(1);
    }

    PIN_ReleaseLock(&lock);
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
    PIN_GetLock(&lock, threadId+1);

    *out << "==========================================================" << endl;
    *out << "                       Thread " << threadId << endl;
    *out << "==========================================================" << endl;
    predictors[threadId]->print(out);

    //uint64_t correct = predictors[threadId]->getCorrectPredictions();
    //uint64_t incorrect = predictors[threadId]->getIncorrectPredictions();
    //double mispredict = 100.0l * (double)incorrect / ((double)correct + (double)incorrect);

    //*out <<  "===============================================" << endl;
    //*out <<  "BranchPredictor analysis results: " << endl;
    //*out <<  "Number of correctly predicted branches: " << correct  << endl;
    //*out <<  "Number of incorrectly predicted branches: " << incorrect  << endl;
    //*out <<  "Branch mispredition rate: " << mispredict << "%" << endl;
    //*out <<  "===============================================" << endl;

    //if (predictor) {
    //    delete predictor;
    //    predictor = 0;
    //}
    PIN_ReleaseLock(&lock);
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
    if (PIN_Init(argc,argv)) {
        return Usage();
    }

    alwaysOn = KnobAlwaysOn.Value();
    PIN_InitLock(&lock);

    for (int i = 0; i < 128; ++i) {
        curTime[i] = 0;
        enabled[i] = alwaysOn;
    }
    updateDelay = KnobUpdateDelay.Value();

    string fileName = KnobOutputFile.Value();
    if (!fileName.empty()) { out = new std::ofstream(fileName.c_str()); }

    INS_AddInstrumentFunction(Branch, 0);
    PIN_AddThreadFiniFunction(Fini, 0);

    PIN_AddThreadStartFunction(threadCreated, 0);
    
    cerr <<  "===============================================" << endl;
    cerr <<  "This application is instrumented by the BranchPredictor Pin Tool" << endl;
    if (!KnobOutputFile.Value().empty()) {
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
