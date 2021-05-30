#ifndef LISTENER_H_
#define LISTENER_H_

#include <stdexcept>
#include <cerrno>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <memory.h>
#include "log/log.h"
#include "utils/utils.h"
#include "event/event.h"
#include "timer/wheel.h"
#include "endpoint/connector.h"


#define DEFAULT_CONNECT_TIMEOUT 2000

class Listener : public Endpoint
{
    private:
        static int timeout_;
        const static int backlog_ = 64;

    private:
        const int fd_;
        const SharedSA rsa_;

    public:
        explicit Listener(const SharedEvent, const SharedSA, const int);
        explicit Listener(const SharedEvent, const SharedSA, const OwnedSA);
        ~Listener() override;
        static void set_timeout(const int);

    public:
        int callback(uint32_t) override;
        int timeout() override;
        int on_accept();
        int inner_fd() const;
};

#endif