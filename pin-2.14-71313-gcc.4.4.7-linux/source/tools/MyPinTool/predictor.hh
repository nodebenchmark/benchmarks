#ifndef __PREDICTOR_HH__
#define __PREDICTOR_HH__

#include "EventQueue.hh"

class Predictor
{
    public:
        typedef uint64_t Addr;
        typedef uint32_t History;
        typedef uint64_t Time;

        class BPUpdateEvent : public Event
        {
            private:
                Predictor *predictor;
                bool result;
                void *state;

            public:
                BPUpdateEvent(Predictor *predictor, Time when,
                        bool result, void *state)
                    : Event(when), predictor(predictor),
                    result(result), state(state)
                {}

                void execute()
                {
                    predictor->update(result, state);
                }
        };

    public:
        Predictor()
            : correct(0), incorrect(0)
        {}

        virtual ~Predictor() {}

        virtual bool predict(Addr addr, void **state_ptr) = 0;

        virtual BPUpdateEvent *createUpdateEvent(Predictor *predictor, Time when, bool result, void *state)
        {
            BPUpdateEvent *event = new BPUpdateEvent(predictor, when, result, state);
            return event;
        }

        virtual uint32_t getHistory(Addr addr) = 0;

        virtual void updateHistory(bool result, void *void_state) {}

        virtual void update(bool result, void *void_state) {}

        uint64_t getCorrectPredictions()
        {
            return correct;
        }

        uint64_t getIncorrectPredictions()
        {
            return incorrect;
        }

    protected:
        uint64_t correct;
        uint64_t incorrect;
};

#endif //__PREDICTOR_HH__
