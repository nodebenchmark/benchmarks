
/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs 
 *  and could serve as the starting point for developing your first PIN tool
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <iterator>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <list>

#include "pin.H"

/* ================================================================== */
// Global variables 
/* ================================================================== */

bool enabled[128];
bool alwaysOn;

UINT64 insCount = 0;        //number of dynamically executed instructions
UINT64 bblCount = 0;        //number of dynamically executed basic blocks
UINT64 threadCount = 0;     //total number of threads, including main thread

std::ostream * out = &cerr;
std::ostream * mapping = &cerr;

typedef std::map<ADDRINT,UINT32> bblFootprint;

std::map<bblFootprint*,UINT32> *footprints;
std::map<bblFootprint*,UINT32> *bigfoot;
std::map<bblFootprint*,UINT32> *prevEvent;

UINT64 prevBytes;
UINT64 prevInsts;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
    "o", "", "specify file name for InstFootprint output");
KNOB<string> KnobMappingFile(KNOB_MODE_WRITEONCE,  "pintool",
    "m", "mapping.out", "specify file name for inst addr to func name mapping");
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

struct ShoeSize {
    UINT64 staticBytes;
    UINT64 staticInsts;
    UINT64 dynamicBytes;
    UINT64 dynamicInsts;
	std::map<ADDRINT, UINT64> instMap;
};
std::list< std::map<ADDRINT, UINT64> > eventsInstMap;
std::set<ADDRINT> global_staticInstsSet;
ShoeSize MeasureFootprint(std::map<bblFootprint*,UINT32> *prints);

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */

/*!
 * Increase counter of the executed basic blocks and instructions.
 * This function is called for every basic block when it is about to be executed.
 * @param[in]   numInstInBbl    number of instructions in the basic block
 * @note use atomic operations for multi-threaded applications
 */
VOID CountBbl(THREADID threadId, VOID *v)
{
    if (!enabled[threadId])
        return;

    bblCount++;
    //insCount += numInstInBbl;

    bblFootprint *toe = (bblFootprint *)v;

    // Count instances of the passed-in BBL footprint
    std::map<bblFootprint*,UINT32>::iterator iter = footprints->find(toe);
    if (iter == footprints->end()) {
        footprints->insert(std::pair<bblFootprint*,UINT32>(toe, 1));
    } else {
        iter->second++;
    }

    // Complete footprint
    std::map<bblFootprint*,UINT32>::iterator big_iter = bigfoot->find(toe);
    if (big_iter == bigfoot->end()) {
        bigfoot->insert(std::pair<bblFootprint*,UINT32>(toe, 1));
    } else {
        big_iter->second++;
    }
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

VOID EnableCounting(THREADID threadId)
{
    assert(!enabled[threadId]);
    enabled[threadId] = true;
}

VOID DisableCounting(THREADID threadId)
{
    assert(enabled[threadId]);
    enabled[threadId] = false;

    ShoeSize shoe = MeasureFootprint(footprints);

    /* for dumping how many times an instruction is referenced */
    //for (std::map<ADDRINT, UINT64>::iterator i = shoe.instMap.begin(),
    //    ie = shoe.instMap.end(); i != ie; ++i) {
	//	*out << i->first << " " << i->second << endl;
	//}
	eventsInstMap.push_back(shoe.instMap);
    footprints->clear();

	/* for dumping static/dynamic/sharing stats */
    //*out << shoe.staticBytes << "," << shoe.dynamicBytes << "," <<
    //    shoe.staticInsts << "," << shoe.dynamicInsts << ",";
    //for (std::map<bblFootprint*,UINT32>::iterator i = footprints->begin(),
    //        ie = footprints->end(); i != ie; ++i) {
    //    prevEvent->erase(i->first);
    //}
    //ShoeSize shoe_overlap = MeasureFootprint(prevEvent);
    //*out << (prevBytes - shoe_overlap.staticBytes) << "," <<
    //    (prevInsts - shoe_overlap.staticInsts) << endl;
    //prevEvent->clear();
    //std::map<bblFootprint*,UINT32> *temp = prevEvent;
    //prevEvent = footprints;
    //footprints = temp;
    //prevBytes = shoe.staticBytes;
    //prevInsts = shoe.staticInsts;
}

map<ADDRINT, string> funcMap;

VOID process_inst(THREADID threadId, ADDRINT ins_addr, VOID *rtn_name)
{
    if (!enabled[threadId])
        return;

	char *func = (char *)rtn_name;

	std::string f(func);
	if(funcMap.find(ins_addr) == funcMap.end()) funcMap[ins_addr] = f;
}

VOID CheckDirectives(INS ins, VOID *v)
{
    // Check for pin_start() and pin_end()
    if (!alwaysOn) {
        if (INS_IsRet(ins)) {
            RTN rtn = INS_Rtn(ins);
            if (RTN_Valid(rtn)) {
                string rtn_name = RTN_Name(rtn);
                if (rtn_name == "pin_start") {
                    INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR) EnableCounting, IARG_THREAD_ID, IARG_END);
                } else if (rtn_name == "pin_end") {
                    INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR) DisableCounting, IARG_THREAD_ID, IARG_END);
                }
            }
        }
    }

    // Insert a call to docount before every instruction, no arguments are passed
    RTN rtn = INS_Rtn(ins);
	string rtn_name;
    if (RTN_Valid(rtn)) rtn_name = RTN_Name(rtn);
	else rtn_name = "invalid_rtn";
	char *p_rtn_name = (char *)malloc((rtn_name.size() + 1) * sizeof(char));
	strcpy(p_rtn_name, rtn_name.c_str());

   	INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)process_inst, IARG_THREAD_ID, IARG_ADDRINT, INS_Address(ins), IARG_PTR, p_rtn_name, IARG_END);
}

/*!
 * Insert call to the CountBbl() analysis routine before every basic block 
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
        bblFootprint *toe = new bblFootprint;
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {
            toe->insert(std::pair<ADDRINT,UINT32>(INS_Address(ins), INS_Size(ins)));
        }

        // Insert a call to CountBbl() before every basic block, passing the number of instructions
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)CountBbl, IARG_THREAD_ID, IARG_PTR, toe, IARG_END);
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

	cout << "Start processing all events" << endl;
	/* dumping inst addr to function name mapping */
	map<ADDRINT, string>::iterator fit;
	for(fit = funcMap.begin();fit != funcMap.end();fit++)
	{
		*mapping << fit->first << " " << fit->second << endl;
	}

	/* One event per row */
	//std::set<ADDRINT>::iterator it;
	//*out << "inst";
	//for(it = global_staticInstsSet.begin();it != global_staticInstsSet.end();it++)
	//{
	//	*out << " " << *it;
	//}
	//*out << endl;

	//UINT64 event_id = 0;
	//while(!eventsInstMap.empty())
	//{
	//	*out << "e_" << event_id++;
	//	std::map<ADDRINT, UINT64> cur_event_instMap = eventsInstMap.front();
	//	eventsInstMap.pop_front();

	//	std::set<ADDRINT>::iterator kt;
	//	for(kt = global_staticInstsSet.begin();kt != global_staticInstsSet.end();kt++)
	//	{
	//		ADDRINT inst = *kt;
	//		if(cur_event_instMap.find(inst) != cur_event_instMap.end())
	//		{
	//			*out << " " << cur_event_instMap[inst];
	//		}
	//		else
	//		{
	//			*out << " 0";
	//		}
	//	}
	//	*out << endl;
	//}

	/* One instruction per row */
	*out << "inst";
	for(unsigned int i = 0;i < eventsInstMap.size();i++)
	{
		*out << " e_" << i;
	}
	*out << endl;

	std::set<ADDRINT>::iterator it;
	for(it = global_staticInstsSet.begin();it != global_staticInstsSet.end();it++)
	{
		ADDRINT inst = *it;
		*out << inst;

		std::list< std::map<ADDRINT, UINT64> >::iterator kt;
		for(kt = eventsInstMap.begin();kt != eventsInstMap.end();kt++)
		{
			if(kt->find(inst) != kt->end())
			{
				*out << " " << (*kt)[inst];
			}
			else
			{
				*out << " 0";
			}
		}
		*out << endl;
	}

	cout << "Finish processing all events" << endl;

    //ShoeSize shoe = MeasureFootprint(bigfoot);
    /* for dumping how many times an instruction is referenced */
    //for (std::map<ADDRINT, UINT64>::iterator i = shoe.instMap.begin(),
    //    ie = shoe.instMap.end(); i != ie; ++i) {
	//	*out << i->first << " " << i->second << endl;
	//}

	/* for dumping static/dynamic/sharing stats */
    //*out << endl;
    //*out << shoe.staticBytes << "," << shoe.dynamicBytes << "," <<
    //    shoe.staticInsts << "," << shoe.dynamicInsts << endl;
}

ShoeSize MeasureFootprint(std::map<bblFootprint*,UINT32> *prints)
{
    UINT64 inst_count = 0;
    ShoeSize shoe;
	std::map<ADDRINT, UINT64> inst_Map; // mapping from the instruction addr to the number of times it's referenced

    std::set<ADDRINT> static_footprint;
    std::set<ADDRINT> static_insts;
    UINT64 dynamic_footprint = 0;
    UINT64 dynamic_insts = 0;

    // Count up the footprint
    for (std::map<bblFootprint*,UINT32>::iterator i = prints->begin(),
            ie = prints->end(); i != ie; ++i) {
        // Count the number of times this basic block was executed
        dynamic_insts += i->second * i->first->size();
        UINT32 dynamic_temp = 0;

        for (bblFootprint::iterator j = i->first->begin(),
                je = i->first->end(); j != je; ++j) {
            // Insert each instruction
            static_insts.insert(j->first);
			global_staticInstsSet.insert(j->first);

            // Insert each byte
            for (UINT32 k = 0; k < j->second; ++k) {
                static_footprint.insert(j->first + k);
            }
            dynamic_temp += j->second;

			UINT64 start_addr = j->first;
			if(inst_Map.find(start_addr) == inst_Map.end())
				inst_Map[start_addr] = 0;
			inst_Map[start_addr] += i->second;
        }

        dynamic_footprint += i->second * dynamic_temp;

        // Double-check the instruction count
        inst_count += i->first->size() * i->second;
    }

    shoe.staticBytes = static_footprint.size();
    shoe.staticInsts = static_insts.size();
    shoe.dynamicBytes = dynamic_footprint;
    shoe.dynamicInsts = dynamic_insts;
    shoe.instMap = inst_Map;

    return shoe;
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

    footprints = new std::map<bblFootprint*,UINT32>;
    bigfoot = new std::map<bblFootprint*,UINT32>;
    prevEvent = new std::map<bblFootprint*,UINT32>;

    alwaysOn = KnobAlwaysOn.Value();
    for (int i = 0; i < 128; ++i) {
        enabled[i] = alwaysOn;
    }
    string fileName = KnobOutputFile.Value();
    string mappingfileName = KnobMappingFile.Value();

    if (!fileName.empty()) { out = new std::ofstream(fileName.c_str());}
    if (!mappingfileName.empty()) { mapping = new std::ofstream(mappingfileName.c_str());}
    //*out << "Static Insts,Dynamic Insts,Static Bytes" << endl;
    //*out << "Static Bytes,Dynamic Bytes,Static Insts,Dynamic Insts,Bytes Shared,Insts Shared" << endl;

    // Register function to be called to instrument traces
    TRACE_AddInstrumentFunction(Trace, 0);

    INS_AddInstrumentFunction(CheckDirectives, 0);

    // Register function to be called for every thread before it starts running
    PIN_AddThreadStartFunction(ThreadStart, 0);

    // Register function to be called when the application exits
    PIN_AddThreadFiniFunction(Fini, 0);

    cerr <<  "===============================================" << endl;
    cerr <<  "This application is instrumented by InstFootprint" << endl;
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
