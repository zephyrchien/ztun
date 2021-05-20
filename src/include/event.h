#ifndef EVENT_H_
#define EVENT_H_

#include <array>
#include <memory>
#include <unordered_set>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstdint>
#include "endpoint.h"
#include "utils.h"


class Event
{
    private:
        const int fd_;
        const static int maxev_ = 1024;
        std::array<epoll_event, maxev_> events_;
    
    public:
        enum STATUS { OK, ERR, CAUTION };

    public:
        explicit Event();
        ~Event();
        int add(int, epoll_event&);
        int add(int, uint32_t, const Endpoint*);
        int mod(int, epoll_event&);
        int mod(int, uint32_t, const Endpoint*);
        int del(int);
        int poll(int);
        int run();
};

typedef std::shared_ptr<Event> SharedEvent;

#endif