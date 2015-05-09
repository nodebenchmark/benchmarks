#include "pin.H"

#include <iostream>
#include <fstream>

#include <stdlib.h>
#include "sched_cache.H"

std::ofstream outFile;

KNOB<UINT32> KnobREPEAT(KNOB_MODE_WRITEONCE , "pintool",
   "repeat", "1", "times to simulate");

struct INST_ADDR_PKG
{
	ADDRINT _addr;
	UINT32 _size;
	INST_ADDR_PKG(ADDRINT addr, UINT32 size): _addr(addr), _size(size) {}
};

namespace IL1
{
    const UINT32 max_sets = 16 * KILO;
    const UINT32 max_associativity = 64;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_NO_ALLOCATE;

   	typedef CACHE_LRU(max_sets, max_associativity, allocation) CACHE;
}

VOID Fini(int code, VOID * v)
{
    outFile.close();
}

int main(int argc, char *argv[])
{
    PIN_Init(argc,argv);

    outFile.open("icache.sched");
	std::ifstream infile("trace.in");

	vector< list<INST_ADDR_PKG> > inst_stream (10);
	IL1::CACHE* il1;
	set<ADDRINT> static_inst;

	int event_num = -1;
	std::string line;
	UINT64 static_size = 0;
	while (std::getline(infile, line))
	{
		if(line.compare("event start") == 0)
		{
			if(event_num != -1) cout << inst_stream[event_num].size() << " insts (static: " << static_inst.size() << ", " << static_size << " B) from event " << event_num << endl;
			event_num++;
			static_inst.clear();
			static_size = 0;
		}
		else
		{
			ADDRINT addr;
			UINT32 size;
			sscanf(line.c_str(), "%lu %u", &addr, &size);
			inst_stream[event_num].push_back(INST_ADDR_PKG(addr, size));
			if(static_inst.find(addr) == static_inst.end())
			{
				static_inst.insert(addr);
				static_size += size;
			}
		}
	}
	cout << inst_stream[event_num].size() << " insts (static: " << static_inst.size() << ", " << static_size << " B) from event " << event_num << endl;
	cout << event_num + 1 << " events parsed" << endl;

	int linesize = 64;
	int assoc = 8;
    il1 = new IL1::CACHE("L1 Instruction Cache",
                         32 * KILO,
                         linesize,
                         assoc,
                         1,
                         32);

	int repeat = KnobREPEAT.Value();
	list<INST_ADDR_PKG>::iterator it;
	cout << "Start simulating " << repeat << " times" << endl;

	UINT64 tot_inst = 0;
	for(int i = 0;i < repeat;i++)
	{
		//int event = 3;
		for(int event = 0;event <= event_num;event++)
		{
			for(it = inst_stream[event].begin();it != inst_stream[event].end();it++)
			{
				tot_inst++;

				INST_ADDR_PKG inst = *it;
    		    const UINT32 size = inst._size;
				const ADDRINT addr = inst._addr;
    		    const BOOL single = (size <= 4);
				if(single) il1->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD);
				else il1->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD);
			}
		}
	}
	cout << "Cache sim done" << endl;

    il1->Instructions(tot_inst);
    outFile <<
        "#\n" <<
        "# ICACHE stats [" << assoc << " (ways), " << linesize << " (bytes)]\n" <<
        "#\n";

    outFile << il1->StatsLong("# ", CACHE_BASE::CACHE_TYPE_ICACHE);
	cout << "Stats dumped" << endl;

    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
