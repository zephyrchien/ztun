#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <array>
#include <memory>
#include <unistd.h>
#include "utils/utils.h"
#include "buffer/buffer.h"


class RingBuffer : public Buffer
{
    public:
        bool ov;
        int r_offset;
        int w_offset;
        const static int size = 0x1000;
        char* data;

        explicit RingBuffer();
        explicit RingBuffer(RingBuffer&&);
        RingBuffer& operator=(RingBuffer&&);
        ~RingBuffer() override;
        char* r_cursor();
        char* w_cursor();
        int xread(const int, int&) override;
        int xwrite(const int, int&) override;
        int xread(const char*, int);
};

#endif