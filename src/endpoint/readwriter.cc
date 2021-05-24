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
    int to_read = 0;
    int n = rbuf_->xread(rfd_, to_read);
    
    if (errno == EWOULDBLOCK || errno == EAGAIN)
    {
        errno = 0;
        int ret = another_->on_write();
        return ret;
    }
    if (to_read == 0)
    {
        int ret = another_->on_write();
        if (ret == Event::OK)
            event_->mod(rfd_, EPOLLIN|EPOLLET, this);
        return ret;
    }
    // EOF
    if (n == 0)
    {
        rbuf_->done = true;
        another_->set_another(nullptr);
        another_->on_write();
        event_->ptrset_.emplace(uintptr_t(this));
        delete this;
        return Event::CAUTION;
    }

    // unexpected error
    errno = 0;
    event_->ptrset_.emplace(uintptr_t(this));
    event_->ptrset_.emplace(uintptr_t(another_));
    delete another_;
    delete this;
    return Event::CAUTION;
}

int ReadWriter::on_write()
{
    int to_write = 0;
    int n __attribute((unused)) = wbuf_->xwrite(wfd_, to_write);

    if (errno == EWOULDBLOCK || errno == EAGAIN)
    {
        errno = 0;
        event_->mod(rfd_, EPOLLOUT|EPOLLET|EPOLLONESHOT, this);
        return Event::OK;
    }
    
    if (to_write == 0)
    {
        if (wbuf_->done)
        {
            event_->ptrset_.emplace(uintptr_t(this));
            delete this;
            return Event::CAUTION;
        }
        return Event::OK;
    }
    errno = 0;
    event_->ptrset_.emplace(uintptr_t(this));
    event_->ptrset_.emplace(uintptr_t(another_));
    delete another_;
    delete this;
    return Event::CAUTION;
}