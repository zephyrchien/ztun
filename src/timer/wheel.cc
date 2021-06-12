#include "wheel.h"


int TimeWheel::intv = DEFAULT_TIMER_INTV;
OwnedTimer TimeWheel::wheel_ = OwnedTimer(nullptr);

TimeWheel::TimeWheel(Event* event, const int fd):
    ev(event), fd_(fd), cursor_(0) { }

TimeWheel::~TimeWheel()
{
    ev->del(fd_);
    close(fd_);
}

OwnedTimer& TimeWheel::instance()
{
    return wheel_;
}

int TimeWheel::init(Event* event)
{
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (fd < 0) return -1;
    itimerspec its;
    its.it_value.tv_sec = intv / 1000;
    its.it_value.tv_nsec = (intv % 1000) * 1000000;
    its.it_interval.tv_sec = intv / 1000;
    its.it_interval.tv_nsec = (intv % 1000) * 1000000;
    if (timerfd_settime(fd, 0, &its, nullptr) < 0)
    {
        close(fd);
        return -1;
    }
    wheel_ = std::unique_ptr<TimeWheel>(new TimeWheel(event, fd));
    wheel_->slots_.fill(nullptr);
    wheel_->ep.callback = [&](uint32_t e) {
        return wheel_->callback(e);
    };
    event->add(fd, EPOLLIN|EPOLLET, &wheel_->ep);
    return 0;
}

Timer* TimeWheel::add(const int timeout, Endpoint* ep, const bool persist)
{
    int ticks = (timeout < intv) ? (1) : (timeout / intv);
    int round = ticks / maxslot;
    int slot = (cursor_ + ticks % maxslot) % maxslot;
    Timer* t = new Timer(timeout, slot, round, persist);
    t->ep = ep;
    // insert to head
    if (slots_[slot] == nullptr) 
        slots_[slot] = t;
    else
    {
        t->next = slots_[slot];
        slots_[slot]->prev = t;
        slots_[slot] = t;
    }
    return t;
}

void TimeWheel::del(const Timer* t)
{
    int slot = t->slot;
    if (t == slots_[slot])
    {
        slots_[slot] = slots_[slot]->next;
        if (slots_[slot] != nullptr)
            slots_[slot]->prev = nullptr;
        delete t;
        return;
    }
    t->prev->next = t->next;
    if (!t->is_tail())
        t->next->prev = t->prev;
    delete t;
}

int TimeWheel::tick()
{
    int ret = 0;
    Timer *t = slots_[cursor_];
    Timer *tx = nullptr;
    while (t != nullptr)
    {
        tx = t;
        if (t->round > 0)
        {
            t->round--;
            t = t->next;
            continue;
        }
        // timeout
        ret = t->ep->on_timeout();
        // delete the timer
        if (t == slots_[cursor_])
        {
            slots_[cursor_] = t->next;
            if (!t->persist) delete t;
            t = slots_[cursor_];
            if (t != nullptr) t->prev = nullptr;
        }else
        {
            t->prev->next = t->next;
            if (!t->is_tail()) t->next->prev = t->prev;
            if (!t->persist) delete tx;
            t = t->next;
        }
        // reload
        if (tx->persist)
        {
            int ticks = (tx->timeout < intv)?
                (1) : (tx->timeout / intv);
            int slot = (cursor_ + ticks % maxslot) % maxslot;
            tx->slot = slot;
            tx->round = ticks / maxslot;
            if (slots_[slot] == nullptr) 
                slots_[slot] = tx;
            else
            {
                tx->next = slots_[slot];
                slots_[slot]->prev = tx;
                slots_[slot] = tx;
            }
        }
    }
    cursor_ = (cursor_ + 1) % maxslot;
    return ret;
}

