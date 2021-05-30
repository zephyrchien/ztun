#ifndef TIME_WHEEL_H_
#define TIME_WHEEL_H_

#include <unistd.h>
#include <sys/timerfd.h>
#include "timer/timer.h"
#include "event/event.h"
#include "endpoint/endpoint.h"


#define DEFAULT_TIMER_INTV 500

class TimeWheel: public Endpoint
{
    private:
        static int intv_;
        const static int maxslot_ = 256;

    private:
        const int fd_;
        int cursor_;
        std::array<Timer*, maxslot_> slots_;

    private:
        explicit TimeWheel(const SharedEvent, const int);
        static OwnedTimer wheel_;

    public:
        ~TimeWheel();
        static int init(const SharedEvent);
        static void set_intv(const int);
        static OwnedTimer& instance();

    public:
        Timer* add(const int, Endpoint*);
        void del(const Timer*);
        int tick();
        int callback(uint32_t) override;
        int timeout() override; 
};

#endif