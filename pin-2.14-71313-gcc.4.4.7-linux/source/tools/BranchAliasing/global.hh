#ifndef __GLOBAL_HH__
#define __GLOBAL_HH__

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>

#include "predictor.hh"
#include "saturating_counter.hh"

class GlobalPredictor: public Predictor
{
    private:
        struct State
        {
            Addr addr;
            History history;
            bool prediction;
        };

    public:
        GlobalPredictor(int size)
            : Predictor(), size(size), history(0), historyMask(size - 1), idxMask(size - 1)
        {
            predictors = new SaturatingCounter[size];
        }

        ~GlobalPredictor()
        {
            if (predictors)
                delete[] predictors;
        }

        bool predict(Addr addr, void **state_ptr)
        {
            *state_ptr = new State;

            Addr idx = getIndex(addr, history);
            assert(idx < (uint64_t)size);
            bool prediction = predictors[idx].read(addr); // The problem is here

            ((State *)*state_ptr)->addr = addr;
            ((State *)*state_ptr)->history = history;
            ((State *)*state_ptr)->prediction = prediction;

            return prediction;
        }

        History getHistory(Addr addr)
        {
            return history;
        }

        void updateHistory(bool result, void *void_state)
        {
            //State *state = (State *)void_state;
            //Addr addr = state->addr;
            //bool prediction = state->prediction;

            // Update the history
            history = ((history << 1) | result) & historyMask;
        }

        void update(bool result, void *void_state)
        {
            State *state = (State *)void_state;
            Addr addr = state->addr;
            bool prediction = state->prediction;

            // Get the index
            Addr idx = getIndex(addr, state->history);
            assert(idx <= (Addr)size);

            // Update the prediction
            if (result)
                ++predictors[idx];
            else
                --predictors[idx];

            // Update the history
            //history = ((history << 1) | result) & historyMask;

            // Keep stats
            if (prediction == result)
                ++correct;
            else
                ++incorrect;

            delete state;
        }

        void clear()
        {
            //history = 0;
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
            *out << "***** Global Predictor *****" << std::endl;
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

        History history;

        SaturatingCounter *predictors;

        History historyMask;

        Addr idxMask;

    private:
        Addr getIndex(Addr addr, History history)
        {
            return history;
        }
};

#endif //__GLOBAL_HH__
