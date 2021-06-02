#include "ringbuffer.h"


RingBuffer::RingBuffer():
    done(false), ov(false),
    r_offset(0), w_offset(0)
{
    data = new char[size];    
}

RingBuffer::~RingBuffer()
{
    delete[] data;
}

char* RingBuffer::r_cursor()
{
    return static_cast<char*>(data + r_offset);
}

char *RingBuffer::w_cursor()
{
    return static_cast<char*>(data + w_offset);
}

int RingBuffer::xread(const int fd, int& to_read)
{
    to_read = ov ? (w_offset - r_offset) : (size - r_offset);
    int n = 0;
    while(to_read > 0)
    {
        n = read(fd, static_cast<char*>(data + r_offset), to_read);
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

int RingBuffer::xwrite(const int fd, int& to_write)
{
    to_write = ov ? (size - w_offset) : (r_offset - w_offset);
    int n = 0;
    while (to_write > 0)
    {
        n = write(fd, static_cast<char*>(data + w_offset), to_write);
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

int RingBuffer::xread(const char *buf, int n)
{
    int to_read = ov ? (w_offset - r_offset) : (size - r_offset);
    if (to_read >= n)
    {
        std::copy_n(buf, n, static_cast<char*>(data + r_offset));
        this->r_offset += n;
    }else if (!ov && (size - r_offset + w_offset) >= n)
    {
        std::copy_n(buf, to_read, static_cast<char*>(data + r_offset));
        this->ov = true;
        this->r_offset = 0;
        std::copy_n(buf + to_read, n - to_read, static_cast<char*>(data + r_offset));
        this->r_offset += (n - to_read);
    }else return -1;
    return 0;
}