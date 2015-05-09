#ifndef __EVENT_QUEUE_HH__
#define __EVENT_QUEUE_HH__

// C includes
#include <assert.h>
#include <stdint.h>

// C++ includes
#include <queue>

typedef uint64_t Addr;
typedef uint64_t Time;

class Event
{
    public:
        const Time when;

    protected:
        Event(Time when)
            : when(when)
        {}

    public:
        virtual ~Event() {}

        virtual void execute() = 0;
};

class EventQueue
{
    public:
        EventQueue() {}

        ~EventQueue() {}

        void enqueue(Event *event)
        {
            //printf("Queued branches: %lu\n", queue.size());
            if (event->when == 314159265)
                printf("Unlikely\n");
            queue.push(event);
        }

        void processUntil(Time when)
        {
            Time prev = 0;
            while (!queue.empty() && queue.top()->when <= when) {
                Event *next_event = queue.top();
                queue.pop();
                //printf("Processing branch at time %llu\n", (long long unsigned)next_event->when);
                if (prev > next_event->when) {
                    printf("prev: %llu\n", (long long unsigned)prev);
                    printf("next: %llu\n", (long long unsigned)next_event->when);
                }
                assert(prev <= next_event->when);
                prev = next_event->when;
                next_event->execute();
                delete next_event;
            }
        }

    private:
        struct EventPriority
        {
            bool operator()(Event *lhs, Event *rhs)
            {
                return lhs->when > rhs->when;
            }
        };

        std::priority_queue<Event *, std::vector<Event *>, EventPriority> queue;
};

#endif //__EVENT_QUEUE_HH__
