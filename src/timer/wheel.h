#ifndef TIME_WHEEL_H_
#define TIME_WHEEL_H_

#include <unistd.h>
#include <sys/timerfd.h>
#include "timer/timer.h"
#include "event/event.h"
#include "endpoint/endpoint.h"


#define DEFAULT_TIMER_INTV 500

class TimeWheel
{
    public:
        static int intv;
        const static int maxslot = 256;

    public:
        Event* ev;
        Endpoint ep;

    private:
        const int fd_;
        int cursor_;
        std::array<Timer*, maxslot> slots_;

    private:
        explicit TimeWheel(Event*, const int);
        static OwnedTimer wheel_;

    public:
        ~TimeWheel();
        static int init(Event*);
        static OwnedTimer& instance();

    public:
        Timer* add(const int, Endpoint*, const bool = false);
        void del(const Timer*);
        int tick();
    
    public:
        inline int callback(uint32_t);
};

inline int TimeWheel::callback(uint32_t event)
{
    if (event & EPOLLIN)
    {
        uint64_t n = 0;
        int x __attribute__((unused)) = read(fd_, &n, sizeof(n));
        for(uint64_t i = 0; i < n; i++)
        {
            tick();
        }
        return 0;
    }
    return -1;
}

#endif