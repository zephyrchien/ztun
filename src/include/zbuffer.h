#ifndef ZBUFFER_H_
#define ZBUFFER_H_

#include <unistd.h>
#include <fcntl.h>
#include "buffer.h"
#include "utils.h"


class ZBuffer : public Buffer
{
    public:
        int rfd;
        int wfd;
        int offset;
        const static int size = 0x10000;

        explicit ZBuffer();
        ~ZBuffer() override; 
        int read(const int, int&) override;
        int write(const int, int&) override;
};

#endif