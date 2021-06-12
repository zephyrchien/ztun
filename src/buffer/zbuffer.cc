#include "zbuffer.h"


ZBuffer* ZBuffer::create()
{
    auto ptr = LinkList<ZBuffer>::alloc();
    return reinterpret_cast<ZBuffer*>(ptr);
}

void ZBuffer::operator delete(void *ptr)
{
    auto buf = static_cast<ZBuffer*>(ptr);
    if (--buf->ref > 0) return;
    LinkList<ZBuffer>::collect(
        static_cast<LinkList<ZBuffer>*>(ptr)
    );
}