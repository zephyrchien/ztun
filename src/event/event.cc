#include "event.h"


Event::Event(): fd_(epoll_create(1)) { }

Event::~Event()
{
    close(fd_);
}

int Event::add(int fd, epoll_event& ev) const
{
    return epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev);
}

int Event::add(int fd, uint32_t events, Endpoint* ep) const
{
    epoll_event ev;
    ev.events = events;
    ev.data.ptr = static_cast<void*>(ep);
    return epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev);
}

int Event::mod(int fd, epoll_event& ev) const
{
    return epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev);
}

int Event::mod(int fd, uint32_t events, Endpoint* ep) const
{
    epoll_event ev;
    ev.events = events;
    ev.data.ptr = static_cast<void*>(ep);
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
    int nready, ev;
    Endpoint* ep;
    while(true)
    {
        nready = epoll_wait(fd_, events_.data(), maxev_, -1);
        if (nready <= 0) continue;
        for (int i=0; i<nready; i++)
        {
            ev = events_[i].events;
            ep = static_cast<Endpoint*>(events_[i].data.ptr);
            ep->callback(ev);
        }
    }
    return 0;
}
