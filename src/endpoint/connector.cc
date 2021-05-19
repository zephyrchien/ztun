#include "connector.h"


Connector::Connector(const SharedEvent event, const int rfd, const int lfd):
    Endpoint(event), rfd_(rfd), lfd_(lfd) { }

Connector::Connector(const SharedEvent event, const SharedSA rsa, const int lfd):
    Endpoint(event), lfd_(lfd), rfd_(socket(AF_INET, SOCK_STREAM, 0))
{
    set_nonblocking(rfd_);
    connect(rfd_, (sockaddr *)rsa.get(), SALEN);
}

Connector::~Connector() { }

int Connector::callback(uint32_t events)
{
    if (events & EPOLLOUT) return on_connect();
    return Event::ERR;
}

int Connector::on_connect()
{
    int error = get_error(rfd_);
    if (error != 0)
    {
        event_->del(lfd_);
        event_->del(rfd_);
        close(lfd_);
        close(rfd_);
        delete this;
        return Event::ERR;
    }
    int lfd2 = dup_with_opt(lfd_);
    int rfd2 = dup_with_opt(rfd_);
    if (lfd2 < 0 || rfd2 < 0)
    {
        event_->del(lfd_);
        event_->del(rfd_);
        close(lfd_);
        close(rfd_);
        close(lfd2);
        close(rfd2);
        delete this;
        return Event::ERR;
    }
    SharedBuffer rbuf = std::make_shared<RingBuffer>();
    SharedBuffer wbuf = std::make_shared<RingBuffer>();
    ReadWriter* rw_fwd = new ReadWriter(event_, lfd_, lfd2, rbuf, wbuf);
    ReadWriter* rw_rev = new ReadWriter(event_, rfd_, rfd2, wbuf, rbuf);
    rw_fwd->set_another(rw_rev);
    rw_rev->set_another(rw_fwd);
    event_->add(lfd_, EPOLLIN|EPOLLET, rw_fwd);
    //event_->add(lfd2, EPOLLOUT|EPOLLET, rw_fwd);
    event_->mod(rfd_, EPOLLIN|EPOLLET, rw_rev);
    //event_->add(rfd2, EPOLLOUT|EPOLLET, rw_rev);
    delete this;
    return Event::OK;
}