#ifndef TIMER_H_
#define TIMER_H_

#include <array>
#include "pool/pool.h"
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
    void* operator new(std::size_t);
    void operator delete(void*);
    inline bool is_head() const { return prev == nullptr; };
    inline bool is_tail() const { return next == nullptr; };
};

#endif