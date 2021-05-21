#include "zbuffer.h"


ZBuffer::ZBuffer(): Buffer(), offset(0)
{
    int rw_pipe[2];
    int ret __attribute__((unused)) = pipe2(rw_pipe, O_NONBLOCK);
    rfd = rw_pipe[0];
    wfd = rw_pipe[1];
}

ZBuffer::~ZBuffer()
{
    close(rfd);
    close(wfd);
}

int ZBuffer::read(const int fd, int& to_read)
{
    int n = 0;
    to_read = size - offset;
    while(to_read > 0)
    {
        n = splice(fd, nullptr, wfd, nullptr, to_read,
            SPLICE_F_MOVE|SPLICE_F_NONBLOCK);
        if (n <= 0) break;
        this->offset += n;
        to_read -= n;
    }
    return n;
}

int ZBuffer::write(const int fd, int& to_write)
{
    int n = 0;
    to_write = offset;
    while(to_write > 0)
    {
        n = splice(rfd, nullptr, fd, nullptr, to_write,
            SPLICE_F_MOVE|SPLICE_F_NONBLOCK|SPLICE_F_MORE);
        if (n <= 0) break;
        this->offset -= n;
        to_write -= n;
    }
    return n;
}