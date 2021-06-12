#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#include <memory>
#include <cerrno>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <memory.h>
#include "log/log.h"
#include "pool/pool.h"
#include "utils/utils.h"
#include "event/event.h"
#include "timer/wheel.h"
#include "buffer/zbuffer.h"
#include "endpoint/endpoint.h"
#include "endpoint/readwriter.h"


class Connector
{
    public:
        Event* ev;
        Endpoint ep;
        Timer* timer;
    
    private:
        const int lfd_;
        const int rfd_;
    
    public:
        explicit Connector(Event*, const int, const int);
        ~Connector();
        void* operator new(std::size_t);
        void operator delete(void*);

    public:
        int on_connect();
        int on_timeout();
        inline int callback(uint32_t);
};

inline int Connector::callback(uint32_t events)
{
    if (events & EPOLLOUT) return on_connect();
    return -1;
}

#endif