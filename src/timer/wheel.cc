#include "wheel.h"


int TimeWheel::intv_ = DEFAULT_TIMER_INTV;
OwnedTimer TimeWheel::wheel_ = OwnedTimer(nullptr);

TimeWheel::TimeWheel(const SharedEvent event, const int fd):
    Endpoint(event), fd_(fd), cursor_(0) { }

TimeWheel::~TimeWheel()
{
    event_->del(fd_);
    close(fd_);
}

void TimeWheel::set_intv(const int intv)
{
    intv_ = intv;
}

OwnedTimer& TimeWheel::instance()
{
    return wheel_;
}

int TimeWheel::init(const SharedEvent event)
{
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (fd < 0) return -1;
    itimerspec its;
    its.it_value.tv_sec = intv_ / 1000;
    its.it_value.tv_nsec = (intv_ % 1000) * 1000000;
    its.it_interval.tv_sec = intv_ / 1000;
    its.it_interval.tv_nsec = (intv_ % 1000) * 1000000;
    if (timerfd_settime(fd, 0, &its, nullptr) < 0)
    {
        close(fd);
        return -1;
    }
    wheel_ = std::unique_ptr<TimeWheel>(new TimeWheel(event, fd));
    wheel_->slots_.fill(nullptr);
    event->add(fd, EPOLLIN|EPOLLET, wheel_.get());
    return 0;
}

Timer* TimeWheel::add(const int timeout, Endpoint* ep)
{
    int ticks = (timeout < intv_) ? (1) : (timeout / intv_);
    int round = ticks / maxslot_;
    int slot = (cursor_ + ticks % maxslot_) % maxslot_;
    Timer* t = new Timer(slot, round);
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
    int ret = Event::OK;
    Timer *t = slots_[cursor_];
    while (t != nullptr)
    {
        if (t->round > 0)
        {
            t->round--;
            t = t->next;
            continue;
        }
        // timeout
        ret = t->ep->timeout();
        // delete the timer
        if (t == slots_[cursor_])
        {
            slots_[cursor_] = t->next;
            delete t;
            t = slots_[cursor_];
            if (t != nullptr) t->prev = nullptr;
            continue;
        }
        t->prev->next = t->next;
        if (!t->is_tail()) t->next->prev = t->prev;
        Timer *tx = t->next;
        delete t;
        t = tx;
    }
    cursor_ = (cursor_ + 1) % maxslot_;
    return ret;
}

int TimeWheel::callback(uint32_t event)
{
    int ret = Event::OK;
    if (event & EPOLLIN)
    {
        uint64_t n = 0;
        int x __attribute__((unused)) = read(fd_, &n, sizeof(n));
        for(uint64_t i = 0; i < n; i++)
        {
            if (tick() == Event::CAUTION)
                ret = Event::CAUTION;
        }
        return ret;
    }
    return Event::ERR;
}

int TimeWheel::timeout()
{
    return 0;
}