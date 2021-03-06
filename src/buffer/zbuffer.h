#ifndef ZBUFFER_H_
#define ZBUFFER_H_

#include <unistd.h>
#include <fcntl.h>
#include "utils/utils.h"
#include "pool/pool.h"

struct ZBuffer
{
    int rfd;
    int wfd;
    int ref;
    int offset;
    const static int size = 0x10000;

    //static ZBuffer* create();
    explicit ZBuffer();
    ~ZBuffer();
    void* operator new(std::size_t);
    void operator delete(void *ptr);
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