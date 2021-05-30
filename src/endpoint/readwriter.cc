#include "readwriter.h"


ReadWriter::ReadWriter(const SharedEvent event, const int rfd,
    const int wfd, const SharedBuffer rbuf, const SharedBuffer wbuf):
    Endpoint(event), rev_(true), wev_(false), rfd_(rfd), wfd_(wfd),
    rbuf_(rbuf), wbuf_(wbuf), another_(nullptr) { }

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

int ReadWriter::timeout()
{
    return 0;
}

int ReadWriter::on_read()
{
    DEBUG("reader[%d]: on read\n", rfd_);

    int to_read = 0;
    int n = rbuf_->xread(rfd_, to_read);
    
    if (to_read == 0)
    {
        DEBUG("reader[%d]: buffer is full\n", rfd_);
        int ret = another_->on_write();
        if (ret == Event::OK)
            event_->mod(rfd_, EPOLLIN|EPOLLET, this);
        return ret;
    }
    if (n < 0 && (errno == EWOULDBLOCK || errno == EAGAIN))
    {
        DEBUG("reader[%d]: socket would block\n", rfd_);
        int ret = another_->on_write();
        return ret;
    }
    // EOF
    if (n == 0)
    {
        DEBUG("reader[%d]: EOF, socket is closed\n", rfd_);
        rbuf_->done = true;
        another_->set_another(nullptr);
        another_->on_write();
        event_->ptrset_.emplace(reinterpret_cast<uintptr_t>(this));
        DEBUG("reader[%d]: delete reader, add to ptrset\n", rfd_);
        delete this;
        return Event::CAUTION;
    }

    // unexpected error
    WARN("reader[%d]: failed, %s\n", rfd_, const_cast<const char*>(strerror(errno)));
    event_->ptrset_.emplace(reinterpret_cast<uintptr_t>(this));
    event_->ptrset_.emplace(reinterpret_cast<uintptr_t>(another_));
    DEBUG("reader[%d]: delete reader and writer, add to ptrset\n", rfd_);
    delete another_;
    delete this;
    return Event::CAUTION;
}

int ReadWriter::on_write()
{
    DEBUG("writer[%d]: on write\n", wfd_);
    int to_write = 0;
    int n = wbuf_->xwrite(wfd_, to_write);

    if (to_write == 0)
    {
        DEBUG("writer[%d]: buffer is empty\n", wfd_);
        if (wbuf_->done)
        {
            DEBUG("writer[%d]: finished, shutdown now\n", wfd_);
            event_->ptrset_.emplace(uintptr_t(this));
            DEBUG("writer[%d]: delete writer, add to ptrset\n", wfd_);
            delete this;
            return Event::CAUTION;
        }
        return Event::OK;
    }
    if (n < 0 && (errno == EWOULDBLOCK || errno == EAGAIN))
    {
        DEBUG("writer[%d]: socket would block\n", wfd_);
        if (!wev_)
        {
            wev_ = true;
            event_->add(wfd_, EPOLLOUT|EPOLLET|EPOLLONESHOT, this);
        }else
            event_->mod(wfd_, EPOLLOUT|EPOLLET|EPOLLONESHOT, this);
        return Event::OK;
    }
    
    WARN("writer[%d]: failed, %s\n", wfd_,
        const_cast<const char*>(strerror(errno)));
    event_->ptrset_.emplace(reinterpret_cast<uintptr_t>(this));
    event_->ptrset_.emplace(reinterpret_cast<uintptr_t>(another_));
    DEBUG("writer[%d]: delete reader and writer, add to ptrset\n", wfd_);
    delete another_;
    delete this;
    return Event::CAUTION;
}