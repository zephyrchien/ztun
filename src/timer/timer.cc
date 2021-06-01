#include "timer.h"


Timer::Timer(const int t, const int s, const int r, const bool p):
    persist(p), timeout(t), slot(s), round(r),
    prev(nullptr), next(nullptr), ep(nullptr) { }

Timer::~Timer() { }

bool Timer::is_head() const
{
    return prev == nullptr;
}

bool Timer::is_tail() const
{
    return next == nullptr;
}
