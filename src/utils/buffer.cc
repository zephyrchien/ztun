#include "buffer.h"


Buffer::Buffer(): ov(false), done(false), r_offset(0), w_offset(0)
{
    data = new char[size];    
}

Buffer::Buffer(Buffer&& buffer)
{
    this->done = buffer.done;
    this->r_offset = buffer.r_offset;
    this->w_offset = buffer.w_offset;
    this->data = buffer.data;
}

Buffer& Buffer::operator=(Buffer&& buffer)
{
    this->done = buffer.done;
    this->r_offset = buffer.r_offset;
    this->w_offset = buffer.w_offset;
    this->data = buffer.data;
    return *this;
}

Buffer::~Buffer()
{
    delete[] data;
}

void* Buffer::r_cursor()
{
    return data + r_offset;
}

void *Buffer::w_cursor()
{
    return data + w_offset;
}