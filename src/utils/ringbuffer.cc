#include "ringbuffer.h"


RingBuffer::RingBuffer(): Buffer(), ov(false), r_offset(0), w_offset(0)
{
    data = new char[size];    
}

RingBuffer::RingBuffer(RingBuffer&& buffer): Buffer()
{
    this->done = buffer.done;
    this->r_offset = buffer.r_offset;
    this->w_offset = buffer.w_offset;
    this->data = buffer.data;
}

RingBuffer& RingBuffer::operator=(RingBuffer&& buffer)
{
    this->done = buffer.done;
    this->r_offset = buffer.r_offset;
    this->w_offset = buffer.w_offset;
    this->data = buffer.data;
    return *this;
}

RingBuffer::~RingBuffer()
{
    delete[] data;
}

void* RingBuffer::r_cursor()
{
    return data + r_offset;
}

void *RingBuffer::w_cursor()
{
    return data + w_offset;
}

int RingBuffer::read(const int fd, int& to_read)
{
    to_read = ov ? (w_offset - r_offset) : (size - r_offset);
    int n = 0;
    while(to_read > 0)
    {
        n = recv(fd, data + r_offset, to_read, 0);
        if (n <= 0) break;
        this->r_offset += n;
        to_read -= n;
        if (r_offset == size)
        {
            this->ov = true;
            this->r_offset = 0;
            to_read = w_offset;
        }
    }
    return n;
}

int RingBuffer::write(const int fd, int& to_write)
{
    to_write = ov ? (size - w_offset) : (r_offset - w_offset);
    int n = 0;
    while (to_write > 0)
    {
        n = send(fd, data + w_offset, to_write, 0);
        if (n <= 0) break;
        this->w_offset += n;
        to_write -= n;
        if (w_offset == size)
        {
            this->ov = false;
            this->w_offset = 0;
            to_write = r_offset;
        }
    }
    return n;
}