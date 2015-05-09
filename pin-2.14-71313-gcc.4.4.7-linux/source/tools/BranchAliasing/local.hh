#ifndef __LOCAL_HH__
#define __LOCAL_HH__

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>

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

            bool prediction = predictors[predictionIdx].read(addr);
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

        void clear()
        {
            //for (int i = 0; i < histories; ++i) {
            //    historyTable[i] = 0;
            //}

            for (int i = 0; i < size; ++i) {
                predictors[i].clear();
            }
        }

        void print(std::ostream *out)
        {
            // Count the aliasing.
            uint64_t aliases_total = 0;
            uint64_t aliases_min = -1;
            uint64_t aliases_max = 0;
            //uint64_t aliases[size];
            for (int i = 0; i < size; ++i) {
                //aliases[i] = predictors[i].countAliases();
                uint64_t aliases_temp = predictors[i].countAliases();
                aliases_total += aliases_temp;
                if (aliases_temp > aliases_max)
                    aliases_max = aliases_temp;
                if (aliases_temp < aliases_min)
                    aliases_min = aliases_temp;
            }

            // Quantify the aliasing.
            uint64_t firstUseBin = 0;
            uint64_t distanceBins[DISTANCE_BINS];
            for (int j = 0; j < size; ++j) {
                firstUseBin += predictors[j].reportAliasesAtDistance(-1);
            }
            for (int i = 0; i < DISTANCE_BINS; ++i) {
                distanceBins[i] = 0;
                for (int j = 0; j < size; ++j) {
                    distanceBins[i] += predictors[j].reportAliasesAtDistance(i);
                }
            }

            double mispredict = 100.0l * (double)incorrect / ((double)correct + (double)incorrect);
            *out << "***** Local Predictor *****" << std::endl;
            *out << "Correctly predicted branches: " << correct << std::endl;
            *out << "Incorrectly predicted branches: " << incorrect << std::endl;
            *out << "Mispredict rate: " << mispredict << "%" << std::endl;
            *out << "Average aliases per prediction: " << ((double)aliases_total / (double)size) << std::endl;
            *out << "Min aliases: " << aliases_min << std::endl;
            *out << "Max aliases: " << aliases_max << std::endl;
            *out << "Alias distribution" << std::endl;
            *out << firstUseBin;
            for (int i = 0; i < DISTANCE_BINS; ++i) {
                *out << "," << distanceBins[i];
            } *out << std::endl;
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
            return addr & historyIdxMask;
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
