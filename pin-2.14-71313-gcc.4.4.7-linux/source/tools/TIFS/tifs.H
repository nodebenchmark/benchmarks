#ifndef __tifs_H__
#define __tifs_H__

// C includes.
#include <assert.h>
#include <stdio.h>

// C++ includes.
#include <list>
#include <map>
#include <queue>
#include <vector>

// Local includes.
#include "pin.H"

// Defines.
#define ASSOCIATIVITY 4
#define IML_LOG_SIZE 8192

class TIFS
{
    private:
        class IndexTable;

        class IML
        {
            private:
                struct IMLEntry
                {
                    bool svbHit;
                    ADDRINT addr;
                };

            private:
                // 8k instruction blocks.
                std::vector<IMLEntry> log;

                // 'log' head and tail pointers.
                int head;
                int tail;

            public:
                IML();

                ~IML();

                int recordMiss(ADDRINT addr, bool svbHit);

                bool validIndex(int idx);

                ADDRINT lookup(int idx);
        };

        class SVB
        {
            private:
                struct SVBEntry
                {
                    bool valid;
                    ADDRINT tag;

                    SVBEntry(ADDRINT tag)
                        : tag(tag)
                    {}
                };

                struct Stream
                {
                    // Prefetch buffers.
                    std::list<SVBEntry> buffer;
                    std::map<ADDRINT,std::list<SVBEntry>::iterator> finder;

                    // FIFO of waiting prefetch requests.
                    //std::queue<ADDRINT> pendingAddrs;

                    // Stream pointer.
                    int imlPointer;

                    void advance()
                    {
                        imlPointer++;
                        imlPointer &= IML_LOG_SIZE - 1;
                    }
                };

            private:
                Stream stream;

                IndexTable *idxTable;

                IML *iml;

            public:
                SVB(IndexTable *table, IML *iml);

                ~SVB();

                bool find(ADDRINT addr);

                void remove(ADDRINT addr);

                void replace(ADDRINT addr);

                void setStream(int idx);

                void prefetch(bool l1_miss);
        };

        class IndexTable
        {
            private:
                std::map<ADDRINT,int> table;

            public:
                IndexTable();

                ~IndexTable();

                void recordMissIndex(ADDRINT addr, int index);

                int lookupIndex(ADDRINT addr);
        };

    private:
        IndexTable indexTable;
        IML iml;
        SVB svb;

    public:
        TIFS();

        ~TIFS();

        void recordMiss(ADDRINT addr, bool prefetched);

        bool check(ADDRINT addr);

        void remove(ADDRINT addr);
};

#endif //__tifs_H__
