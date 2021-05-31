#ifndef READWRITER_H_
#define READWRITER_H_

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cerrno>
#include "log/log.h"
#include "utils/utils.h"
#include "event/event.h"
#include "buffer/ringbuffer.h"


class ReadWriter : public Endpoint
{
    private:
        bool rev_, wev_;
        const int rfd_;
        const int wfd_;
        const SharedBuffer rbuf_;
        const SharedBuffer wbuf_;
        ReadWriter* another_;

    public:
        explicit ReadWriter(Event*, const int, const int,
            const SharedBuffer, const SharedBuffer);
        ~ReadWriter() override;
        int inner_rfd() const;
        int inner_wfd() const;
        SharedBuffer inner_rbuf() const;
        SharedBuffer inner_wbuf() const;
        void set_another(ReadWriter*);
        int callback(uint32_t) override;
        int timeout() override;
        int on_read();
        int on_write();
};

#endif