#include "readwriter.h"


ReadWriter::ReadWriter(const SharedEvent event, const int rfd,
    const int wfd, const SharedBuffer rbuf, const SharedBuffer wbuf):
    Endpoint(event), rfd_(rfd), wfd_(wfd),
    rbuf_(rbuf), wbuf_(wbuf), another_(nullptr) { }

ReadWriter::ReadWriter(const ReadWriter& rw, const int rfd,
    const int wfd):
    Endpoint(rw.inner_event()), rfd_(rfd), wfd_(wfd),
    rbuf_(rw.inner_wbuf()), wbuf_(rw.inner_rbuf()),
    another_(nullptr) { }

ReadWriter::~ReadWriter()
{
    event_->del(rfd_);
    event_->del(wfd_);
    close(rfd_);
    close(wfd_);
}

int ReadWriter::inner_rfd() const
{
    return rfd_;
}

int ReadWriter::inner_wfd() const
{
    return wfd_;
}

SharedBuffer ReadWriter::inner_rbuf() const
{
    return rbuf_;
}

SharedBuffer ReadWriter::inner_wbuf() const
{
    return wbuf_;
}

void ReadWriter::set_another(ReadWriter* another)
{
    another_ = another;
}


int ReadWriter::callback(uint32_t events)
{
    if (events & EPOLLIN) return on_read();
    if (events & EPOLLOUT) return on_write();
    return Event::ERR;
}

int ReadWriter::on_read()
{
    DEBUG("reader: on read\n");

    int to_read = 0;
    int n = rbuf_->xread(rfd_, to_read);
    
    if (errno == EWOULDBLOCK || errno == EAGAIN)
    {
        DEBUG("reader: socket would block\n");
        errno = 0;
        int ret = another_->on_write();
        return ret;
    }
    if (to_read == 0)
    {
        DEBUG("reader: buffer is full\n");
        int ret = another_->on_write();
        if (ret == Event::OK)
            event_->mod(rfd_, EPOLLIN|EPOLLET, this);
        return ret;
    }
    // EOF
    if (n == 0)
    {
        DEBUG("reader: EOF, socket is closed\n");
        rbuf_->done = true;
        another_->set_another(nullptr);
        another_->on_write();
        event_->ptrset_.emplace(reinterpret_cast<uintptr_t>(this));
        DEBUG("reader: delete reader, add to ptrset\n");
        delete this;
        return Event::CAUTION;
    }

    // unexpected error
    WARN("reader: failed, %s", const_cast<const char*>(strerror(errno)));
    errno = 0;
    event_->ptrset_.emplace(reinterpret_cast<uintptr_t>(this));
    event_->ptrset_.emplace(reinterpret_cast<uintptr_t>(another_));
    DEBUG("reader: delete reader and writer, add to ptrset\n");
    delete another_;
    delete this;
    return Event::CAUTION;
}

int ReadWriter::on_write()
{
    DEBUG("writer: on write\n");
    int to_write = 0;
    int n __attribute((unused)) = wbuf_->xwrite(wfd_, to_write);

    if (errno == EWOULDBLOCK || errno == EAGAIN)
    {
        DEBUG("writer: socket would block\n");
        errno = 0;
        event_->mod(rfd_, EPOLLOUT|EPOLLET|EPOLLONESHOT, this);
        return Event::OK;
    }
    
    if (to_write == 0)
    {
        DEBUG("writer: buffer is empty\n");
        if (wbuf_->done)
        {
            DEBUG("writer: finished, shutdown now\n");
            event_->ptrset_.emplace(uintptr_t(this));
            DEBUG("writer: delete writer, add to ptrset\n");
            delete this;
            return Event::CAUTION;
        }
        return Event::OK;
    }

    WARN("writer: failed, %s\n", const_cast<const char*>(strerror(errno)));
    errno = 0;
    event_->ptrset_.emplace(uintptr_t(this));
    event_->ptrset_.emplace(uintptr_t(another_));
    DEBUG("writer: delete reader and writer, add to ptrset\n");
    delete another_;
    delete this;
    return Event::CAUTION;
}