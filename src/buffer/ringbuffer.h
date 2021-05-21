#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <array>
#include <memory>
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
        void* r_cursor();
        void* w_cursor();
        int read(const int, int&) override;
        int write(const int, int&) override;
};

#endif