#ifndef TIMER_H_
#define TIMER_H_

#include <array>
#include "endpoint/endpoint.h"

struct Timer
{
    bool persist;
    const int timeout;

    int slot;
    int round;
    Timer* prev;
    Timer* next;
    Endpoint* ep;

    explicit Timer(const int, const int, const int,
        const bool = false);
    ~Timer();
    bool is_head() const;
    bool is_tail() const;
};

#endif