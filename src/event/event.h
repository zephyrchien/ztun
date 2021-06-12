#ifndef EVENT_H_
#define EVENT_H_

#include <array>
#include <memory>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstdint>
#include "utils/utils.h"
#include "endpoint/endpoint.h"


class Event
{
    private:
        const int fd_;
        const static int maxev_ = 1024;
        std::array<epoll_event, maxev_> events_;

    public:
        explicit Event();
        ~Event();
        int add(int, epoll_event&) const;
        int add(int, uint32_t, Endpoint*) const;
        int mod(int, epoll_event&) const;
        int mod(int, uint32_t, Endpoint*) const;
        int del(int) const;
        int poll(int);
        int run();
};

typedef std::shared_ptr<Event> SharedEvent;

#endif