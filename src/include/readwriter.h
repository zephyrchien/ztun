#ifndef READWRITER_H_
#define READWRITER_H_

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cerrno>
#include "event.h"
#include "ringbuffer.h"
#include "endpoint.h"
#include "utils.h"


class ReadWriter : public Endpoint
{
    private:
        const int rfd_;
        const int wfd_;
        const SharedBuffer rbuf_;
        const SharedBuffer wbuf_;
        ReadWriter* another_;

    public:
        explicit ReadWriter(const SharedEvent, const int, const int,
            const SharedBuffer, const SharedBuffer);
        explicit ReadWriter(const ReadWriter&, const int, const int);
        ~ReadWriter() override;
        int inner_rfd() const;
        int inner_wfd() const;
        SharedBuffer inner_rbuf() const;
        SharedBuffer inner_wbuf() const;
        void set_another(ReadWriter*);
        int callback(uint32_t) override;
        int on_read();
        int on_write();
};

#endif