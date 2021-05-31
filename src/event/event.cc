#include "event.h"


Event::Event(): fd_(epoll_create(1))
{
    ptrset_.reserve(20);
}

Event::~Event()
{
    close(fd_);
}

int Event::add(int fd, epoll_event& ev) const
{
    return epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev);
}

int Event::add(int fd, uint32_t events, const Endpoint* ep) const
{
    epoll_event ev;
    ev.events = events;
    ev.data.ptr = (void*)ep;
    return epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev);
}

int Event::mod(int fd, epoll_event& ev) const
{
    return epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev);
}

int Event::mod(int fd, uint32_t events, const Endpoint* ep) const
{
    epoll_event ev;
    ev.events = events;
    ev.data.ptr = (void*)ep;
    return epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev);
}

int Event::del(int fd) const
{
    return epoll_ctl(fd_, EPOLL_CTL_DEL, fd, nullptr);
}

int Event::poll(int timeout)
{
    return epoll_wait(fd_, events_.data(), maxev_, timeout);
}

int Event::run()
{
    int nready, ev, ret;
    bool need_skip = false;
    Endpoint* ep;
    while(true)
    {
        if (need_skip)
        {
            ptrset_.clear();
            need_skip = false;
        }
        if ((nready = epoll_wait(fd_, events_.data(), maxev_, -1)) <= 0) continue;
        for (int i=0; i<nready; i++)
        {
            ev = events_[i].events;
            ep = (Endpoint *)events_[i].data.ptr;
            if (need_skip && ptrset_.find(uintptr_t(ep)) != ptrset_.end())
            {
                continue;
            }
            ret = ep->callback(ev);
            if (ret == Event::CAUTION) need_skip = true;
        }
    }
    delete this;
    return 0;
}