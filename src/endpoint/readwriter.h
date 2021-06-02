#ifndef READWRITER_H_
#define READWRITER_H_

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cerrno>
#include "log/log.h"
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

    private:
        bool rev_, wev_;
        const int rfd_;
        const int wfd_;
        const SharedZBuffer rbuf_;
        const SharedZBuffer wbuf_;

    public:
        explicit ReadWriter(Event*, const int, const int,
            const SharedZBuffer, const SharedZBuffer);
        ~ReadWriter();

        int callback(uint32_t) ;
        int on_read();
        int on_write();
};

#endif