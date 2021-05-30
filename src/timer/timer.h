#ifndef TIMER_H_
#define TIMER_H_

#include <array>
#include "endpoint/endpoint.h"

struct Timer
{
    const int slot;
    int round;
    Endpoint* ep;
    Timer* prev;
    Timer* next;

    explicit Timer(const int, const int);
    ~Timer();
    bool is_head() const;
    bool is_tail() const;
};

#endif