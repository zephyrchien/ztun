#include "readwriter.h"


ReadWriter::ReadWriter(Event* event,
    const int rfd, const int wfd,
    ZBuffer* rbuf, ZBuffer* wbuf):
    ev(event), another(nullptr), rev(false), wev(false),
    rfd_(rfd), wfd_(wfd), rbuf_(rbuf), wbuf_(wbuf){ }

ReadWriter::~ReadWriter()
{
    ev->del(rfd_);
    ev->del(wfd_);
    close(rfd_);
    close(wfd_);
    delete rbuf_;
    delete wbuf_;
}

void* ReadWriter::operator new(std::size_t size)
{
    auto ptr = LinkList<ReadWriter>::alloc();
    return ptr;
}

void ReadWriter::operator delete(void *ptr)
{
    auto rw = static_cast<ReadWriter*>(ptr);
    rw->ep = Pool::empty_ep;
    LinkList<ReadWriter>::collect(
        static_cast<LinkList<ReadWriter>*>(ptr)
    );
}

int ReadWriter::on_read()
{
    DEBUG("reader[%d]: on read\n", rfd_);

    int to_read = 0;
    int n = rbuf_->xread(rfd_, to_read);
    
    if (to_read == 0)
    {
        DEBUG("reader[%d]: buffer is full\n", rfd_);
        int ret = another->on_write();
        if (ret == 0)
            ev->mod(rfd_, EPOLLIN|EPOLLET, &ep);
        return ret;
    }
    if (n < 0 && (errno == EWOULDBLOCK || errno == EAGAIN))
    {
        DEBUG("reader[%d]: socket would block\n", rfd_);
        int ret = another->on_write();
        return ret;
    }
    // EOF
    if (n == 0)
    {
        DEBUG("reader[%d]: EOF, socket is closed\n", rfd_);
        another->rev = true;
        another->another = nullptr;
        another->on_write();
        delete this;
        return 0;
    }

    // unexpected error
    WARN("reader[%d]: failed, %s\n", rfd_,
        const_cast<const char*>(strerror(errno)));
    delete another;
    delete this;
    return -1;
}

int ReadWriter::on_write()
{
    DEBUG("writer[%d]: on write\n", wfd_);
    int to_write = 0;
    int n = wbuf_->xwrite(wfd_, to_write);

    if (to_write == 0)
    {
        DEBUG("writer[%d]: buffer is empty\n", wfd_);
        if (rev)
        {
            DEBUG("writer[%d]: finished, shutdown now\n", wfd_);
            delete this;
            return 1;
        }
        return 0;
    }
    if (n < 0 && (errno == EWOULDBLOCK || errno == EAGAIN))
    {
        DEBUG("writer[%d]: socket would block\n", wfd_);
        if (!wev)
        {
            wev = true;
            ev->add(wfd_, EPOLLOUT|EPOLLET|EPOLLONESHOT, &ep);
        }else
            ev->mod(wfd_, EPOLLOUT|EPOLLET|EPOLLONESHOT, &ep);
        return 0;
    }
    
    WARN("writer[%d]: failed, %s\n", wfd_,
        const_cast<const char*>(strerror(errno)));
    delete another;
    delete this;
    return -1;
}