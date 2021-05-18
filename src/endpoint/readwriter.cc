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
    int to_read = rbuf_->ov ?
        (rbuf_->w_offset - rbuf_->r_offset) :
        (rbuf_->size - rbuf_->r_offset);
    int n = 0;
    while(to_read > 0)
    {
        n = recv(rfd_, rbuf_->r_cursor(), to_read, 0);
        if (n <= 0) break;
        rbuf_->r_offset += n;
        to_read -= n;
        if (rbuf_->r_offset == rbuf_->size)
        {
            rbuf_->ov = true;
            rbuf_->r_offset = 0;
            to_read = rbuf_->w_offset;
        }
    }
    
    if (errno == EWOULDBLOCK || errno == EAGAIN)
    {
        errno = 0;
        another_->on_write();
        return Event::OK;
    }
    if (to_read == 0)
    {
        another_->on_write();
        event_->mod(rfd_, EPOLLIN|EPOLLET, this);
        return Event::OK;
    }
    // EOF
    if (n == 0)
    {
        rbuf_->done = true;
        another_->set_another(nullptr);
        another_->on_write();
        delete this;
        return Event::CAUTION;
    }

    // unexpected error
    errno = 0;
    delete another_;
    delete this;
    return Event::CAUTION;
}

int ReadWriter::on_write()
{
    int to_write = wbuf_->ov ? 
        (wbuf_->size - wbuf_->w_offset):
        (wbuf_->r_offset - wbuf_->w_offset);
    int n = 0;
    while (to_write > 0)
    {
        n = send(wfd_, wbuf_->w_cursor(), to_write, 0);
        if (n <= 0) break;
        wbuf_->w_offset += n;
        to_write -= n;
        if (wbuf_->w_offset == wbuf_->size)
        {
            wbuf_->ov = false;
            wbuf_->w_offset = 0;
            to_write = wbuf_->r_offset;
        }
    }

    if (errno == EWOULDBLOCK || errno == EAGAIN)
    {
        errno = 0;
        return Event::OK;
    }
    
    if (to_write == 0)
    {
        if (wbuf_->done)
        {
            delete this;
            return Event::CAUTION;
        }
        return Event::OK;
    }
    errno = 0;
    delete another_;
    delete this;
    return Event::CAUTION;
}