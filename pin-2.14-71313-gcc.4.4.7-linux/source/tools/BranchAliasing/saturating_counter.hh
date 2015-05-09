#ifndef __SATURATING_COUNTER_HH__
#define __SATURATING_COUNTER_HH__

#include <stdint.h>

#include <fstream>
#include <iostream>
#include <list>
#include <map>

#define DISTANCE_BINS 257

class SaturatingCounter
{
    private:
        class BranchAddrHistory
        {
            private:
                std::list<uint64_t> branchHistory;

                std::pair<int,std::list<uint64_t>::iterator> find(uint64_t addr)
                {
                    std::list<uint64_t>::iterator curr = branchHistory.begin(),
                        end = branchHistory.end();
                    int distance = 0;
                    while (curr != end) {
                        if (*curr == addr) {
                            break;
                        } else {
                            ++curr;
                            ++distance;
                        }
                    }

                    if (curr == end)
                        distance = -1;
                    return std::pair<int,std::list<uint64_t>::iterator>(distance, curr);
                }

            public:
                BranchAddrHistory() {}

                int distance(uint64_t addr)
                {
                    std::pair<int,std::list<uint64_t>::iterator> last = find(addr);
                    std::list<uint64_t>::iterator iter = last.second;

                    // Update the history.
                    if (iter != branchHistory.end())
                        branchHistory.erase(iter);
                    branchHistory.push_front(addr);

                    return last.first;
                }

                void clear()
                {
                    branchHistory.clear();
                }
        };

    public:
        SaturatingCounter()
            : bits(2), init(1), value(1), maxValue(3)
        {}

        SaturatingCounter(int bits, int init)
            : bits(bits), init(init), value(init)
        {
            maxValue = (1 << bits) - 1;
        }

        void increment()
        {
            if (value < maxValue)
                ++value;
        }

        void decrement()
        {
            if (value)
                --value;
        }

        bool read(uint64_t addr)
        {
            std::map<uint64_t,uint64_t>::iterator iter = branches.find(addr);
            if (iter == branches.end()) {
                branches.insert(std::pair<uint64_t,uint64_t>(addr, 1));
            } else {
                ++iter->second;
            }

            // Keep track of intervening branches.
            int distance = branchHistory.distance(addr);
            if (distance > (DISTANCE_BINS-1))
                distance = (DISTANCE_BINS-1);
            if (distance == -1)
                ++firstUseBin;
            else
                ++distanceBins[distance];

            return value >> (bits-1);
        }

        // Pre-increment
        bool operator++() { increment(); return read(); }

        // Post-increment
        bool operator++(int) {
            bool temp = read();
            increment();
            return temp;
        }

        // Pre-decrement
        bool operator--() { decrement(); return read(); }

        // Post-decrement
        bool operator--(int) {
            bool temp = read();
            decrement();
            return temp;
        }

        void clear()
        {
            branchHistory.clear();
            firstUseBin = 0;
            for (int i = 0; i < DISTANCE_BINS; ++i) {
                distanceBins[i] = 0;
            }
        }

        uint64_t countAliases() {
            // Total the dynamic uses of this predictor.
            //uint64_t total = 0;
            //for (std::map<uint64_t,uint64_t>::iterator i = branches.begin(),
            //        ie = branches.end(); i != ie; ++i) {
            //    total += i->second;
            //}

            // Report the aliasing.
            //for (std::map<uint64_t,uint64_t>::iterator i = branches.begin(),
            //        ie = branches.end(); i != ie; ++i) {
            //}
            return branches.size();
        }

        uint64_t reportAliasesAtDistance(int distance) {
            if (distance == -1)
                return firstUseBin;
            else
                return distanceBins[distance];
        }

    private:
        int bits;
        int init;

        int value;
        int maxValue;

        std::map<uint64_t,uint64_t> branches;

        BranchAddrHistory branchHistory;

        uint64_t firstUseBin;
        uint64_t distanceBins[DISTANCE_BINS];

        bool read() {
            return value >> (bits-1);
        }
};

#endif //__SATURATING_COUNTER_HH__
