#include "event.h"


Event::Event(): fd_(epoll_create(1)) { }

Event::~Event()
{
    close(fd_);
}

int Event::add(int fd, epoll_event& ev)
{
    return epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev);
}

int Event::add(int fd, uint32_t events, const Endpoint* ep)
{
    epoll_event ev;
    ev.events = events;
    ev.data.ptr = (void*)ep;
    return epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev);
}

int Event::mod(int fd, epoll_event& ev)
{
    return epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev);
}

int Event::mod(int fd, uint32_t events, const Endpoint* ep)
{
    epoll_event ev;
    ev.events = events;
    ev.data.ptr = (void*)ep;
    return epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev);
}

int Event::del(int fd)
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
    bool need_skip;
    Endpoint* ep;
    std::unordered_set<uintptr_t> destroyed;
    destroyed.reserve(20);
    while(true)
    {
        need_skip = false;
        if ((nready = epoll_wait(fd_, events_.data(), maxev_, -1)) <= 0) continue;
        for (int i=0; i<nready; i++)
        {
            ev = events_[i].events;
            ep = (Endpoint *)events_[i].data.ptr;
            if (need_skip && destroyed.find(uintptr_t(ep))!=destroyed.end())
            {
                continue;
            }
            ret = ep->callback(ev);
            if (ret == Event::CAUTION)
            {
                need_skip = true;
                destroyed.emplace(uintptr_t(ep));
            }
        }
    }
    return 0;
}