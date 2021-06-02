#ifndef LISTENER_H_
#define LISTENER_H_

#include <stdexcept>
#include <functional>
#include <cerrno>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <memory.h>
#include "log/log.h"
#include "utils/utils.h"
#include "event/event.h"
#include "timer/wheel.h"
#include "endpoint/endpoint.h"
#include "endpoint/connector.h"


#define DEFAULT_CONNECT_TIMEOUT 2000

class Listener
{
    public:
        Event* ev;
        Endpoint ep;

    public:
        static int timeout;
        const static int backlog = 64;

    private:
        const int fd_;
        const sa_family_t family_;
        const addrinfo* hints_;

    public:
        explicit Listener(Event*, const addrinfo*,
            const sa_family_t, const sockaddr_storage*);
        ~Listener();
        int inner_fd() const;

    public:
        int on_accept();
        int callback(uint32_t);
};

#endif