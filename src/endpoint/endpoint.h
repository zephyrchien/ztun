#ifndef ENDPOINT_H_
#define ENDPOINT_H_

#include <memory>
#include <array>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "utils/utils.h"
#include "event/event.h"


class Endpoint
{
    protected:
        Event* event_;

    public:
        explicit Endpoint(Event*);
        virtual ~Endpoint();
        virtual int callback(uint32_t) = 0;
        virtual int timeout() = 0;
        const Event* inner_event() const;
};

#endif