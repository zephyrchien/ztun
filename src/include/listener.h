#ifndef LISTENER_H_
#define LISTENER_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <memory.h>
#include "event.h"
#include "endpoint.h"
#include "connector.h"
#include "utils.h"


class Listener : public Endpoint
{
    private:
        const int fd_;
        const static int backlog_ = 64;
        const SharedSA rsa_;

    public:
        explicit Listener(const SharedEvent, const SharedSA, const int);
        explicit Listener(const SharedEvent, const SharedSA, const OwnedSA);
        ~Listener() override;
        int callback(uint32_t) override;
        int on_accept();
        int inner_fd() const;
};

#endif