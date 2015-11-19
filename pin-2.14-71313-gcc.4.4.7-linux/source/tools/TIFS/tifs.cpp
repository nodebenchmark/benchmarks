#include "tifs.H"

/************************************************
 ******************     IML      ****************
 ************************************************/

TIFS::IML::IML()
    : log(IML_LOG_SIZE), head(0), tail(0)
{
}

TIFS::IML::~IML()
{
}

int
TIFS::IML::recordMiss(ADDRINT addr, bool svbHit)
{
    if (tail == head)
        if (++head == IML_LOG_SIZE)
            head = 0;

    // Keep track of the index where the miss is recorded.
    int index = tail;

    log[tail].addr = addr;
    log[tail].svbHit = svbHit;
    if (++tail == IML_LOG_SIZE)
        tail = 0;

    return index;
}

bool
TIFS::IML::validIndex(int idx)
{
    return true;
}

ADDRINT
TIFS::IML::lookup(int idx)
{
    return log[idx].addr | log[idx].svbHit;;
}

/************************************************
 ******************     SVB      ****************
 ************************************************/

TIFS::SVB::SVB(IndexTable *table, IML *iml)
    : idxTable(table), iml(iml)
{
}

TIFS::SVB::~SVB()
{
}

bool
TIFS::SVB::find(ADDRINT addr)
{
    std::map<ADDRINT,std::list<SVBEntry>::iterator>::iterator iter = stream.finder.find(addr);
    if (iter == stream.finder.end())
        return false;

    /*// Update LRU ordering.
    stream.buffer.erase(iter->second);
    stream.buffer.push_front(SVBEntry(addr));
    iter->second = stream.buffer.begin();*/

    return true;
}

void
TIFS::SVB::remove(ADDRINT addr)
{
    std::map<ADDRINT,std::list<SVBEntry>::iterator>::iterator iter = stream.finder.find(addr);
    assert(iter != stream.finder.end());

    // Remove the relevant entry.
    stream.buffer.erase(iter->second);
    stream.finder.erase(iter);
}

void
TIFS::SVB::replace(ADDRINT addr)
{
    std::map<ADDRINT,std::list<SVBEntry>::iterator>::iterator iter = stream.finder.find(addr);
    assert(iter == stream.finder.end());

    if (stream.finder.size() == ASSOCIATIVITY) {
        // Replace and update LRU ordering.
        stream.finder.erase(stream.buffer.back().tag);
        stream.buffer.pop_back();
    } else {
        assert(stream.finder.size() < ASSOCIATIVITY);
    }

    stream.buffer.push_front(SVBEntry(addr));
    stream.finder.insert(std::pair<ADDRINT,std::list<SVBEntry>::iterator>(addr, stream.buffer.begin()));
    assert(stream.finder.size() <= ASSOCIATIVITY);
}

void
TIFS::SVB::setStream(int idx)
{
    stream.imlPointer = idx;
}

void
TIFS::SVB::prefetch(bool l1_miss)
{
    int idx = stream.imlPointer;
    ADDRINT next_block = iml->lookup(idx);
    bool stream_end = !(next_block & 0x1);
    next_block = next_block & ~((ADDRINT)0x1);

    // Only prefetch if we're not at the end of a stream.
    if (l1_miss || !stream_end) {
        // Don't prefetch if it's already in the SVB.
        if (!find(next_block)) {
            replace(next_block);
        }
    }

    // Update the pointer.
    stream.advance();
}

/************************************************
 ******************  Index Table ****************
 ************************************************/

TIFS::IndexTable::IndexTable()
{
}

TIFS::IndexTable::~IndexTable()
{
}

void
TIFS::IndexTable::recordMissIndex(ADDRINT addr, int index)
{
    table[addr] = index;
    //assert(table.size() <= 8192);
}

int
TIFS::IndexTable::lookupIndex(ADDRINT addr)
{
    std::map<ADDRINT,int>::iterator iter = table.find(addr);
    if (iter == table.end())
        return -1;

    return iter->second;
}

/************************************************
 ******************     TIFS     ****************
 ************************************************/

TIFS::TIFS()
    : svb(&indexTable, &iml)
{
}

TIFS::~TIFS()
{
}

void
TIFS::recordMiss(ADDRINT addr, bool prefetched)
{
    indexTable.recordMissIndex(addr, iml.recordMiss(addr, prefetched));
}

bool
TIFS::check(ADDRINT addr)
{
    bool svb_hit = svb.find(addr);

    if (svb_hit) {
        //printf("Hit!\n");
        svb.remove(addr);
        svb.prefetch(false);
    } else {
        int miss_idx = indexTable.lookupIndex(addr);
        if (miss_idx != -1) {
            svb.setStream((miss_idx + 1) & (IML_LOG_SIZE - 1));
            svb.prefetch(true);
        }
    }

#if 0
    // Prefetch the next block.
    int miss_idx = indexTable.lookupIndex(addr);
    int next_idx = (miss_idx + 1) & 8191;
    if (miss_idx != -1) {
        ADDRINT next_block = iml.lookup(next_idx);
        /*bool svb_hit = next_block & 0x1;*/
        next_block &= ~((ADDRINT)0x1);

        // Only fetch the next block if this block was an SVB hit last time.
        //if (true || svb_hit) {
        if (!svb.find(next_block)) {
            svb.replace(next_block);
        }
    }
#endif

    return svb_hit;
}

void
TIFS::remove(ADDRINT addr)
{
}
