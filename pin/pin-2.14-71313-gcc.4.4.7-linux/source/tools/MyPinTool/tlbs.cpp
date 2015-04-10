#include <assert.h>
#include <stdio.h>
#include "pin.H"
#include <stdlib.h>
#include <tr1/unordered_map>
#include <vector>
#include <cstring>
#include <iostream>
#include <fstream>
typedef unsigned char BYTE;   

#define warmup 5000000000
#define fwd 0//5000000000//10000000000//1000000000//40000000000
#define simlength /*10000000000000*/5000000000

ofstream OutFile;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
        "o", "tlbsim.out", "specify output file name");
KNOB<UINT32> KnobL1TlbSets(KNOB_MODE_WRITEONCE, "pintool",
        "l1-sets", "16", "number of sets in L1 TLB");
KNOB<UINT32> KnobL1TlbWays(KNOB_MODE_WRITEONCE, "pintool",
        "l1-ways", "4", "number of ways in L1 TLB");
KNOB<UINT32> KnobL2TlbSets(KNOB_MODE_WRITEONCE, "pintool",
        "l2-sets", "128", "number of sets in L2 TLB");
KNOB<UINT32> KnobL2TlbWays(KNOB_MODE_WRITEONCE, "pintool",
        "l2-ways", "4", "number of ways in L2 TLB");
KNOB<UINT32> KnobWatch(KNOB_MODE_WRITEONCE, "pintool",
        "watch", "1", "whether to watch for flags to enable/disable the tool");
static UINT64 icount[128];
bool enabled[128];
bool watch;

unsigned long dtlbmissl1[128];
ADDRINT **dtlb_l1_tag[128]; //[16][4];
unsigned int **dtlb_l1_info[128]; //[16][4];
bool **dtlb_l1_valid[128]; //[16][4];

unsigned long itlbmissl1[128];
ADDRINT **itlb_l1_tag[128]; //[16][4];
unsigned int **itlb_l1_info[128]; //[16][4];
bool **itlb_l1_valid[128]; //[16][4];

unsigned long tlbmissl2;
ADDRINT **tlb_l2_tag; //[128][4];
unsigned int **tlb_l2_info; //[128][4];
bool **tlb_l2_valid; //[128][4];

ADDRINT mmu_l4_tag[2];
unsigned int mmu_l4_info[2];
bool mmu_l4_valid[2];
unsigned int mmu_l4_count[2];
unsigned int mmu_l4_access;
unsigned int mmu_l4_hit;

ADDRINT mmu_l3_tag[4];
unsigned int mmu_l3_info[4];
bool mmu_l3_valid[4];
unsigned int mmu_l3_count[4];
unsigned int mmu_l3_access;
unsigned int mmu_l3_hit;

ADDRINT mmu_l2_tag[32];
unsigned int mmu_l2_info[32];
bool mmu_l2_valid[32];
unsigned int mmu_l2_count[32];
unsigned int mmu_l2_access;
unsigned int mmu_l2_hit;

VOID access_tlb_l2(ADDRINT addr);
VOID access_mmu(ADDRINT addr);

unsigned l1Sets;
unsigned l1Ways;
unsigned l1SetMask;
unsigned l1WaysMax;

/* ===================================================================== */

unsigned int CalculateL1Set(ADDRINT addr)
{
    return (addr>>12) & l1SetMask;
}

/* ===================================================================== */

VOID EnableTLB(THREADID threadId)
{
    assert(!enabled[threadId]);
    enabled[threadId] = true;
	cout << "event start" << endl;
}

/* ===================================================================== */

VOID DisableTLB(THREADID threadId)
{
    assert(enabled[threadId]);
    enabled[threadId] = false;
	cout << "event stop" << endl;
}

/* ===================================================================== */

VOID CountInst(THREADID threadId)
{
    if (!enabled[threadId])
        return;

    icount[threadId]++;
}

/* ===================================================================== */

VOID tlb_l1_init(THREADID threadId, CONTEXT *ctxt, INT32 flags, VOID *v){
    UINT32 sets = KnobL1TlbSets.Value();
    //UINT32 ways = KnobL1TlbWays.Value();

    dtlbmissl1[threadId]=0;
    dtlb_l1_valid[threadId] = new bool*[sets];
    dtlb_l1_info[threadId] = new unsigned int*[sets];
    dtlb_l1_tag[threadId] = new ADDRINT*[sets];
    for(unsigned i=0; i<sets; i++){
        dtlb_l1_valid[threadId][i] = new bool[l1Ways];
        dtlb_l1_info[threadId][i] = new unsigned int[l1Ways];
        dtlb_l1_tag[threadId][i] = new ADDRINT[l1Ways];
        for (unsigned j=0; j<l1Ways; j++){
            dtlb_l1_valid[threadId][i][j]=false;
            dtlb_l1_info[threadId][i][j]=j;	
        }
    }

    itlbmissl1[threadId]=0;
    itlb_l1_valid[threadId] = new bool*[sets];
    itlb_l1_info[threadId] = new unsigned int*[sets];
    itlb_l1_tag[threadId] = new ADDRINT*[sets];
    for(unsigned i=0; i<sets; i++){
        itlb_l1_valid[threadId][i] = new bool[l1Ways];
        itlb_l1_info[threadId][i] = new unsigned int[l1Ways];
        itlb_l1_tag[threadId][i] = new ADDRINT[l1Ways];
        for (unsigned j=0; j<l1Ways; j++){
            itlb_l1_valid[threadId][i][j]=false;
            itlb_l1_info[threadId][i][j]=j;	
        }
    }
}

VOID tlb_l2_init(){
    UINT32 sets = KnobL2TlbSets.Value();
    UINT32 ways = KnobL2TlbWays.Value();
    tlbmissl2=0;
    tlb_l2_valid = new bool*[sets];
    tlb_l2_info = new unsigned int*[sets];
    tlb_l2_tag = new ADDRINT*[sets];
    for(unsigned i=0; i<sets; i++){
        tlb_l2_valid[i] = new bool[ways];
        tlb_l2_info[i] = new unsigned int[ways];
        tlb_l2_tag[i] = new ADDRINT[ways];
        for (unsigned j=0; j<ways; j++){
            tlb_l2_valid[i][j]=false;	
            tlb_l2_info[i][j]=j;	
        }
    }
}

VOID mmu_init(){
    for(int i=0; i<2; i++){
        mmu_l4_valid[i]=false;
        mmu_l4_count[i]=0;
        mmu_l4_info[i]=i;	
    }
    mmu_l4_access=0;
    mmu_l4_hit=0;
    for(int i=0; i<4; i++){
        mmu_l3_valid[i]=false;
        mmu_l3_count[i]=0;
        mmu_l3_info[i]=i;	
    }
    mmu_l3_access=0;
    mmu_l3_hit=0;
    for(int i=0; i<32; i++){
        mmu_l2_valid[i]=false;
        mmu_l2_count[i]=0;
        mmu_l2_info[i]=i;	
    }
    mmu_l2_access=0;
    mmu_l2_hit=0;
}

VOID access_itlb_l1(THREADID threadId, ADDRINT addr){
    if (!enabled[threadId])
        return;

    //UINT32 ways = KnobL1TlbWays.Value();

    unsigned int set, vic;
    set = CalculateL1Set(addr);
    bool hit;
    hit =false;
    vic=0;
    for(unsigned int i=0; i<l1Ways; i++){
        if(itlb_l1_valid[threadId][set][i]){
            if((itlb_l1_tag[threadId][set][i]>>12)==(addr>>12)){
                hit=true;
                vic=i;
                break;
            }
        }
    }
    if(!hit){
        itlbmissl1[threadId]++;
        //access_tlb_l2(addr);
        for(unsigned int i=0; i<l1Ways; i++){
            if(itlb_l1_info[threadId][set][i]==l1WaysMax){
                vic=i;
                itlb_l1_valid[threadId][set][i]=true;
                itlb_l1_tag[threadId][set][i]=addr;
                break;
            }
        }
    }
    for(unsigned int i=0; i<l1Ways; i++){
        if(itlb_l1_info[threadId][set][i]<itlb_l1_info[threadId][set][vic]){
            itlb_l1_info[threadId][set][i]++;
        }
    }
    itlb_l1_info[threadId][set][vic]=0;
}

VOID access_dtlb_l1(THREADID threadId, ADDRINT addr){
    if (!enabled[threadId])
        return;

    //UINT32 ways = KnobL1TlbWays.Value();

    unsigned int set, vic;
    set = CalculateL1Set(addr);
    bool hit;
    hit =false;
    vic=0;
    for(unsigned int i=0; i<l1Ways; i++){
        if(dtlb_l1_valid[threadId][set][i]){
            if((dtlb_l1_tag[threadId][set][i]>>12)==(addr>>12)){
                hit=true;
                vic=i;
                break;
            }
        }
    }
    if(!hit){
        dtlbmissl1[threadId]++;
        //access_tlb_l2(addr);
        for(unsigned int i=0; i<l1Ways; i++){
            if(dtlb_l1_info[threadId][set][i]==l1WaysMax){
                vic=i;
                dtlb_l1_valid[threadId][set][i]=true;
                dtlb_l1_tag[threadId][set][i]=addr;
                break;
            }
        }
    }
    for(unsigned int i=0; i<l1Ways; i++){
        if(dtlb_l1_info[threadId][set][i]<dtlb_l1_info[threadId][set][vic]){
            dtlb_l1_info[threadId][set][i]++;
        }
    }
    dtlb_l1_info[threadId][set][vic]=0;
}

VOID access_tlb_l2(ADDRINT addr){
    unsigned int set, vic;
    set=(addr>>12)&0x7f;
    bool hit;
    hit =false;
    vic=0;
    for(int i=0; i<4; i++){
        if(tlb_l2_valid[set][i]){
            if((tlb_l2_tag[set][i]>>12)==(addr>>12)){
                hit=true;
                vic=i;
                break;
            }
        }
    }
    if(!hit){
        tlbmissl2++;
        access_mmu(addr);
        for(int i=0; i<4; i++){
            if(tlb_l2_info[set][i]==(4-1)){
                vic=i;
                tlb_l2_valid[set][i]=true;
                tlb_l2_tag[set][i]=addr;
                break;
            }
        }
    }
    for(int i=0; i<4; i++){
        if(tlb_l2_info[set][i]<tlb_l2_info[set][vic]){
            tlb_l2_info[set][i]++;
        }
    }
    tlb_l2_info[set][vic]=0;
}

VOID access_mmu(ADDRINT addr){
    bool hitl4, hitl3, hitl2;
    hitl4 =false;
    hitl3 =false;
    hitl2 =false;
    unsigned int vicl4, vicl3, vicl2;
    vicl4=0;
    vicl3=0;
    vicl2=0;
    for(int i=0; i<2; i++){
        if(mmu_l4_valid[i]){
            if( ((mmu_l4_tag[i]>>39)&0x1ff) == ((addr>>39)&0x1ff) ){
                hitl4=true;
                vicl4=i;
                break;
            }
        }
    }
    for(int i=0; i<4; i++){
        if(mmu_l3_valid[i]){
            if( ((mmu_l3_tag[i]>>30)&0x3ffff) == ((addr>>30)&0x3ffff) ){
                hitl3=true;
                vicl3=i;
                break;
            }
        }
    }
    for(int i=0; i<32; i++){
        if(mmu_l2_valid[i]){
            if( ((mmu_l2_tag[i]>>21)&0x7ffffff) == ((addr>>21)&0x7ffffff) ){
                hitl2=true;
                vicl2=i;
                break;
            }
        }
    }
    if( (!hitl2) && (!hitl3) ){
        mmu_l4_access++;
    }
    if(!hitl4){
        for(int i=0; i<2; i++){
            if(mmu_l4_info[i]==(2-1)){
                vicl4=i;
                mmu_l4_valid[i]=true;
                mmu_l4_tag[i]=addr;
                break;
            }
        }
    }else{
        if( (!hitl2) && (!hitl3) ){
            mmu_l4_hit++;
        }
    }

    if(!hitl2){
        mmu_l3_access++;
    }
    if(!hitl3){
        for(int i=0; i<4; i++){
            if(mmu_l3_info[i]==(4-1)){
                vicl3=i;
                mmu_l3_valid[i]=true;
                mmu_l3_tag[i]=addr;
                break;
            }
        }
    }else{
        if(!hitl2){
            mmu_l3_hit++;
        }
    }

    mmu_l2_access++;
    if(!hitl2){
        for(int i=0; i<32; i++){
            if(mmu_l2_info[i]==(32-1)){
                vicl2=i;
                mmu_l2_valid[i]=true;
                mmu_l2_tag[i]=addr;
                break;
            }
        }
    }else{
        mmu_l2_hit++;
    }

    for(int i=0; i<2; i++){
        if(mmu_l4_info[i]<mmu_l4_info[vicl4]){
            mmu_l4_info[i]++;
        }
    }
    mmu_l4_info[vicl4]=0;
    mmu_l4_count[vicl4]++;

    for(int i=0; i<4; i++){
        if(mmu_l3_info[i]<mmu_l3_info[vicl3]){
            mmu_l3_info[i]++;
        }
    }
    mmu_l3_info[vicl3]=0;
    mmu_l3_count[vicl3]++;

    for(int i=0; i<32; i++){
        if(mmu_l2_info[i]<mmu_l2_info[vicl2]){
            mmu_l2_info[i]++;
        }
    }
    mmu_l2_info[vicl2]=0;
    mmu_l2_count[vicl2]++;
}

VOID docount_internal1(THREADID threadId)
{
    OutFile<<"Thread " << threadId << std::endl;
    OutFile<<"ins "<<icount[threadId]<<endl;
	OutFile<<"iTLB miss L1 "<<itlbmissl1[threadId]<<endl;
    OutFile<<"dTLB miss L1 "<<dtlbmissl1[threadId]<<endl;
    OutFile<<"L1 iTLB MPKI "<<(double)itlbmissl1[threadId] / ((double)icount[threadId] / 1000.00) <<endl;
    OutFile<<"L1 dTLB MPKI "<<(double)dtlbmissl1[threadId] / ((double)icount[threadId] / 1000.00) <<endl<<endl;
    //OutFile<<"TLB miss L2 "<<tlbmissl2<<endl;
    //OutFile<<"Access L2 "<<mmu_l2_access<<" Hit L2 "<<mmu_l2_hit<<endl;
    //OutFile<<"Access L3 "<<mmu_l3_access<<" Hit L3 "<<mmu_l3_hit<<endl;
    //OutFile<<"Access L4 "<<mmu_l4_access<<" Hit L4 "<<mmu_l4_hit<<endl;
}

// This function is called before every block
VOID docount(THREADID threadId, ADDRINT addr, UINT32 size) { 
    if (!size)
        return;

    const ADDRINT orig = addr;
    const ADDRINT highAddr = addr + size;
    const ADDRINT notOffsetMask = ~((1 << 12) - 1);
    uint64_t iterations = 0;
    do
    {
        if (++iterations >= 3)
            printf("addr: %#lx, originally %#lx, size is %d, waiting for %#lx\n", addr, orig, size, highAddr);
        access_itlb_l1(threadId, addr);
        addr = (addr & notOffsetMask) + (ADDRINT)(1 << 12); // start of next page
    }
    while (addr < highAddr && addr >= orig);

}



// Print a memory read record
VOID RecordMemRead(THREADID threadId, VOID * ip, ADDRINT addr, UINT32 size)
{
    if (!size)
        return;

    const ADDRINT orig = addr;
    const ADDRINT highAddr = addr + size;
    const ADDRINT notOffsetMask = ~((1 << 12) - 1);
    uint64_t iterations = 0;
    do
    {
        if (++iterations >= 3)
            printf("addr: %#lx, originally %#lx, size is %d, waiting for %#lx\n", addr, orig, size, highAddr);
        access_dtlb_l1(threadId, addr);
        addr = (addr & notOffsetMask) + (ADDRINT)(1 << 12); // start of next page
    }
    while (addr < highAddr && addr >= orig);

    //if(((UINT32)size>0)&&((ADDRINT)addr>0)){
    //    if(icount[threadId]>(UINT64)fwd){
    //        access_dtlb_l1(threadId, addr);
    //    }
    //}
}

// Print a memory write record
VOID RecordMemWrite(THREADID threadId, VOID * ip, ADDRINT addr, UINT32 size)
{
    if (!size)
        return;

    const ADDRINT orig = addr;
    const ADDRINT highAddr = addr + size;
    const ADDRINT notOffsetMask = ~((1 << 12) - 1);
    do
    {
        access_dtlb_l1(threadId, addr);
        addr = (addr & notOffsetMask) + (1 << 12); // start of next page
    }
    while (addr < highAddr && addr >= orig);

    //if(((UINT32)size>0)&&((ADDRINT)addr>0)){
    //    if(icount[threadId]>(UINT64)fwd){ 
    //        access_dtlb_l1(threadId, addr);
    //    }
    //}
}

//---------------------------------
//	
//   Instrument on Instruction-Level
//
//---------------------------------
// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
{
    // Instruments memory accesses using a predicated call, i.e.
    // the instrumentation is called iff the instruction will actually be executed.
    //
    // The IA-64 architecture has explicitly predicated instructions. 
    // On the IA-32 and Intel(R) 64 architectures conditional moves and REP 
    // prefixed instructions appear as predicated instructions in Pin.


    // Check for pin_start() and pin_end()
    if (watch) {
        if (INS_IsRet(ins)) {
            RTN rtn = INS_Rtn(ins);
            if (RTN_Valid(rtn)) {
                string rtn_name = RTN_Name(rtn);
                if (rtn_name == "pin_start") {
                    INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR) EnableTLB, IARG_THREAD_ID, IARG_END);
                    cout << "start found" << endl;
                } else if (rtn_name == "pin_end") {
                    INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR) DisableTLB, IARG_THREAD_ID, IARG_END);
                    cout << "stop found" << endl;
                }
            }
        }
    }

    // Insert a call to docount before every instruction, no arguments are passed
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_THREAD_ID, IARG_ADDRINT, INS_Address(ins), IARG_UINT32, INS_Size(ins), IARG_END);		// increase #count before each instruction is execute
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CountInst, IARG_THREAD_ID, IARG_END);
    UINT32 memOperands = INS_MemoryOperandCount(ins); 
    // Instrument instruction on a operand-level
    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            if(INS_HasFallThrough(ins)){
                INS_InsertPredicatedCall(
                        ins, IPOINT_AFTER, (AFUNPTR)RecordMemRead,
                        IARG_THREAD_ID,
                        IARG_INST_PTR,
                        IARG_MEMORYOP_EA, 
                        memOp,
                        IARG_MEMORYREAD_SIZE,
                        IARG_END);
            }else{
                INS_InsertPredicatedCall(
                        ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                        IARG_THREAD_ID,
                        IARG_INST_PTR,
                        IARG_MEMORYOP_EA, 
                        memOp,
                        IARG_MEMORYREAD_SIZE,
                        IARG_END);
            }
        }

        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            if(INS_HasFallThrough(ins)){
                INS_InsertPredicatedCall(
                        ins, IPOINT_AFTER, (AFUNPTR)RecordMemWrite,
                        IARG_THREAD_ID,
                        IARG_INST_PTR,
                        IARG_MEMORYOP_EA,
                        memOp,
                        IARG_MEMORYWRITE_SIZE,
                        IARG_END);
            }else{
                INS_InsertPredicatedCall(
                        ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                        IARG_THREAD_ID,
                        IARG_INST_PTR,
                        IARG_MEMORYOP_EA,
                        memOp,
                        IARG_MEMORYWRITE_SIZE,
                        IARG_END);
            }
        }
    }
}

VOID ThreadFini(THREADID threadId, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    docount_internal1(threadId);
}

VOID Fini(INT32 code, VOID *v)
{
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR( "This Pintool is a tlb sim\n" 
            + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv)) return Usage();
    watch = KnobWatch.Value();
    for (int i = 0; i < 128; ++i) {
        icount[i] = 0;
        enabled[i] = !watch;
    }
    l1Sets = KnobL1TlbSets.Value();
    l1SetMask = l1Sets - 1;
    l1Ways = KnobL1TlbWays.Value();
    l1WaysMax = l1Ways-1;
    PIN_AddThreadStartFunction(tlb_l1_init, 0);
    //tlb_l1_init();
    //tlb_l2_init();
    //mmu_init();
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);
    PIN_AddFiniFunction(Fini, 0);

    OutFile.open(KnobOutputFile.Value().c_str());
    OutFile.setf(ios::showbase);

    // Never returns
    PIN_StartProgram();

    return 0;
}
