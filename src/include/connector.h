#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#include <memory>
#include <cerrno>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <memory.h>
#include "event.h"
#include "ringbuffer.h"
#include "endpoint.h"
#include "readwriter.h"
#include "utils.h"


class Connector : public Endpoint
{
    private:
        const int lfd_;
        const int rfd_;
    
    public:
        explicit Connector(const SharedEvent, const int, const int);
        explicit Connector(const SharedEvent, const SharedSA, const int);
        ~Connector() override;
        int inner_fd() const;
        int callback(uint32_t) override;
        int on_connect();
};

#endif