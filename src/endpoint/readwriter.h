#ifndef READWRITER_H_
#define READWRITER_H_

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cerrno>
#include "log/log.h"
#include "pool/pool.h"
#include "utils/utils.h"
#include "event/event.h"
#include "buffer/zbuffer.h"
#include "endpoint/endpoint.h"


class ReadWriter
{
    public:
        Event* ev;
        Endpoint ep;
        ReadWriter* another;

    public:
        bool rev, wev;

    private:
        const int rfd_;
        const int wfd_;
        ZBuffer* rbuf_;
        ZBuffer* wbuf_;

    public:
        explicit ReadWriter(Event*, const int, const int,
            ZBuffer*, ZBuffer*);
        ~ReadWriter();
        void* operator new(std::size_t);
        void operator delete(void*);

    public:
        inline int callback(uint32_t) ;
        int on_read();
        int on_write();
};

inline int ReadWriter::callback(uint32_t events)
{
    if (events & EPOLLIN) return on_read();
    if (events & EPOLLOUT) return on_write();
    return -1;
}

#endif