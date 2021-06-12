#include "zbuffer.h"

/**
ZBuffer* ZBuffer::create()
{
    auto ptr = LinkList<ZBuffer>::alloc();
    return reinterpret_cast<ZBuffer*>(ptr);
}
*/

ZBuffer::ZBuffer(): ref(2), offset(0)
{
    int rw_pipe[2];
    int ret __attribute__((unused)) = pipe2(rw_pipe, O_NONBLOCK);
    rfd = rw_pipe[0];
    wfd = rw_pipe[1];
}

ZBuffer::~ZBuffer() { }

void* ZBuffer::operator new(std::size_t size)
{
    auto ptr = LinkList<ZBuffer>::alloc();
    return ptr;
}

void ZBuffer::operator delete(void *ptr)
{
    auto buf = static_cast<ZBuffer*>(ptr);
    if (--buf->ref > 0) return;
    close(buf->rfd);
    close(buf->wfd);
    LinkList<ZBuffer>::collect(
        static_cast<LinkList<ZBuffer>*>(ptr)
    );
}