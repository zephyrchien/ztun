#ifndef ZBUFFER_H_
#define ZBUFFER_H_

#include <unistd.h>
#include <fcntl.h>
#include "utils/utils.h"
#include "buffer/buffer.h"


class ZBuffer : public Buffer
{
    public:
        int rfd;
        int wfd;
        int offset;
        const static int size = 0x10000;

        explicit ZBuffer();
        ~ZBuffer() override; 
        int xread(const int, int&) override;
        int xwrite(const int, int&) override;
};

#endif