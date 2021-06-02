#ifndef ZBUFFER_H_
#define ZBUFFER_H_

#include <unistd.h>
#include <fcntl.h>
#include "utils/utils.h"


struct ZBuffer
{
    bool done;
    int rfd;
    int wfd;
    int offset;
    const static int size = 0x10000;

    explicit ZBuffer();
    ~ZBuffer();
    inline int xread(const int, int&);
    inline int xwrite(const int, int&);
};

inline int ZBuffer::xread(const int fd, int& to_read)
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

inline int ZBuffer::xwrite(const int fd, int& to_write)
{
    int n = 0;
    to_write = offset;
    while(to_write > 0)
    {
        n = splice(rfd, nullptr, fd, nullptr, to_write,
            SPLICE_F_MOVE|SPLICE_F_NONBLOCK);
        if (n <= 0) break;
        this->offset -= n;
        to_write -= n;
    }
    return n;
}

#endif