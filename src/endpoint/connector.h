#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#include <memory>
#include <cerrno>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <memory.h>
#include "log/log.h"
#include "utils/utils.h"
#include "event/event.h"
#include "timer/wheel.h"
#include "buffer/ringbuffer.h"
#include "buffer/zbuffer.h"
#include "endpoint/readwriter.h"


class Connector : public Endpoint
{
    private:
        const int lfd_;
        const int rfd_;
        Timer* timer_;
    
    public:
        explicit Connector(Event*, const int, const int);
        ~Connector() override;
        void set_timer(Timer*);
        int callback(uint32_t) override;
        int timeout() override;
        int on_connect();
};

#endif