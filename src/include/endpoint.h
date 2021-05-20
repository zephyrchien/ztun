#ifndef ENDPOINT_H_
#define ENDPOINT_H_

#include <memory>
#include <array>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "event.h"
#include "utils.h"


class Endpoint
{
    protected:
        const SharedEvent event_;

    public:
        explicit Endpoint(const SharedEvent);
        virtual ~Endpoint();
        virtual int callback(uint32_t, PtrSet&) = 0;
        SharedEvent inner_event() const;
};

#endif