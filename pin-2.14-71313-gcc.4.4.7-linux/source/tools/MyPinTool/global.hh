#ifndef __GLOBAL_HH__
#define __GLOBAL_HH__

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

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
            : Predictor(), size(size), historyMask(size - 1), idxMask(size - 1)
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
            bool prediction = predictors[idx].read();

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
