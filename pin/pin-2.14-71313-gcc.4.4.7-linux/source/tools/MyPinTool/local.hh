#ifndef __LOCAL_HH__
#define __LOCAL_HH__

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "predictor.hh"
#include "saturating_counter.hh"

class LocalPredictor: public Predictor
{
    private:
        struct State
        {
            Addr addr;
            History history;
            bool prediction;
        };

    public:
        LocalPredictor(int size, int histories)
            : Predictor(), size(size), historyBits(log2(size)),
            histories(histories), idxMask(size - 1),
            historyMask(size - 1), historyIdxMask(histories - 1)
        {
            predictors = new SaturatingCounter[size];
            historyTable = new History[histories];
            for (int i = 0; i < histories; ++i) {
                historyTable[i] = 0;
            }
        }

        ~LocalPredictor()
        {
            if (predictors)
                delete[] predictors;
            if (historyTable)
                delete[] historyTable;
        }

        bool predict(Addr addr, void **state_ptr)
        {
            *state_ptr = new State;

            // Get the indices
            Addr historyIdx = getIndex(addr);
            Addr predictionIdx = historyTable[historyIdx];

            bool prediction = predictors[predictionIdx].read();
            ((State *)*state_ptr)->prediction = prediction;
            ((State *)*state_ptr)->history = predictionIdx;
            ((State *)*state_ptr)->addr = addr;

            return prediction;
        }

        History getHistory(Addr addr)
        {
            return getIndex(addr);
        }

        void updateHistory(bool result, void *void_state)
        {
            State *state = (State *)void_state;
            Addr addr = state->addr;

            // Get the index
            Addr historyIdx = getIndex(addr);
            assert(historyIdx <= (Addr)histories);

            // Update the history
            historyTable[historyIdx] = (historyTable[historyIdx] << 1) | result;
            historyTable[historyIdx] &= historyMask;
        }

        void update(bool result, void *void_state)
        {
            State *state = (State *)void_state;
            //Addr addr = state->addr;
            bool prediction = state->prediction;

            // Get the indices
            //Addr historyIdx = getIndex(addr);
            //assert(historyIdx <= (Addr)histories);
            //Addr predictionIdx = historyTable[historyIdx];
            Addr predictionIdx = state->history;
            assert(predictionIdx <= (Addr)size);

            // Update the prediction
            if (result)
                ++predictors[predictionIdx];
            else
                --predictors[predictionIdx];

            //// Update the history
            //historyTable[historyIdx] = (historyTable[historyIdx] << 1) | result;
            //historyTable[historyIdx] &= historyMask;

            // Keep stats
            if (prediction == result)
                ++correct;
            else
                ++incorrect;

            delete state;
        }

    private:
        int size;

        int historyBits;

        int histories;

        Addr idxMask;

        History historyMask;

        uint32_t historyIdxMask;

        SaturatingCounter *predictors;

        History *historyTable;

    private:
        Addr getIndex(Addr addr)
        {
            return (addr >> 2) & historyIdxMask;
        }

        uint64_t log2(uint64_t value)
        {
            uint64_t result = 0;
            while (value > 1) {
                ++result;
                value >>= 1;
            }

            return result;
        }
};

#endif //__LOCAL_HH__
