#include "timer.h"


Timer::Timer(const int s, const int r):
    slot(s), round(r),ep(nullptr),
    prev(nullptr), next(nullptr) { }

Timer::~Timer() { }

bool Timer::is_head() const
{
    return prev == nullptr;
}

bool Timer::is_tail() const
{
    return next == nullptr;
}
