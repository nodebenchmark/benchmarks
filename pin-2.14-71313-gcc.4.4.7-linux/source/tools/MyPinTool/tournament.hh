#ifndef __TOURNAMENT_HH__
#define __TOURNAMENT_HH__

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "global.hh"
#include "local.hh"
#include "predictor.hh"
#include "saturating_counter.hh"

class TournamentPredictor : public Predictor
{
    private:
        struct State
        {
            Addr addr;
            History history;
            bool localPred;
            bool globalPred;
            bool prediction;

            void *localState;

            void *globalState;
        };

    public:
        TournamentPredictor(int local_size,
                int local_histories, int global_size, int tournament_size)
            : Predictor(), localPredictor(local_size, local_histories),
            globalPredictor(global_size), size(tournament_size), history(0),
            historyMask(size - 1)
        {
            predictors = new SaturatingCounter[size];
        }

        ~TournamentPredictor()
        {
            if (predictors)
                delete[] predictors;
        }

        bool predict(Addr addr, void **state_ptr)
        {
            *state_ptr = new State;
            State *state = (State *)*state_ptr;

            Addr idx = getIndex(addr, history);
            bool decision = predictors[idx].read();

            bool local_pred = localPredictor.predict(addr, &state->localState);
            bool global_pred = globalPredictor.predict(addr, &state->globalState);
            state->history = history;
            state->localPred = local_pred;
            state->globalPred = global_pred;

            bool prediction = decision ? local_pred : global_pred;

            state->prediction = prediction;
            return prediction;
        }

        History getHistory(Addr addr)
        {
            return history;
        }

        void updateHistory(bool result, void *void_state)
        {
            State *state = (State *)void_state;
            //Addr addr = state->addr;
            //bool prediction = state->prediction;

            localPredictor.updateHistory(result, state->localState);
            globalPredictor.updateHistory(result, state->globalState);

            // Update the history
            history = ((history << 1) | result) & historyMask;
        }

        void update(bool result, void *void_state)
        {
            State *state = (State *)void_state;
            Addr addr = state->addr;
            bool prediction = state->prediction;

            localPredictor.update(result, state->localState);
            globalPredictor.update(result, state->globalState);

            // Get the index
            Addr idx = getIndex(addr, state->history);
            assert(idx <= (Addr)size);

            // Update the prediction
            bool local_correct = state->localPred == result;
            bool global_correct = state->globalPred == result;
            if (local_correct && !global_correct) {
                ++predictors[idx];
            } else if (global_correct && !local_correct) {
                --predictors[idx];
            }

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
        LocalPredictor localPredictor;
        GlobalPredictor globalPredictor;

        int size;

        SaturatingCounter *predictors;

        History history;

        History historyMask;

    private:
        Addr getIndex(Addr addr, History history)
        {
            return history;
        }
};

#endif //__TOURNAMENT_HH__
