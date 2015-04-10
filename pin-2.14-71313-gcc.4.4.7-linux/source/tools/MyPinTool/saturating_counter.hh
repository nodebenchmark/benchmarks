#ifndef __SATURATING_COUNTER_HH__
#define __SATURATING_COUNTER_HH__

class SaturatingCounter
{
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

        bool read() { return value >> (bits-1); }

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

    private:
        int bits;
        int init;

        int value;
        int maxValue;
};

#endif //__SATURATING_COUNTER_HH__
