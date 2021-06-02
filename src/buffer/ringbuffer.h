#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <array>
#include <memory>
#include <unistd.h>
#include "utils/utils.h"


struct RingBuffer
{
    bool done;
    bool ov;
    int r_offset;
    int w_offset;
    const static int size = 0x1000;
    char* data;

    explicit RingBuffer();
    ~RingBuffer();
    char* r_cursor();
    char* w_cursor();
    int xread(const int, int&);
    int xwrite(const int, int&);
    int xread(const char*, int);
};

#endif