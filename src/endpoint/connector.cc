#include "connector.h"


Connector::Connector(const SharedEvent event, const int rfd, const int lfd):
    Endpoint(event), lfd_(lfd), rfd_(rfd) { }

Connector::~Connector() { }

int Connector::callback(uint32_t events)
{
    if (events & EPOLLOUT) return on_connect();
    return Event::ERR;
}

int Connector::on_connect()
{
    DEBUG("connector[%d-%d]: on connect\n", lfd_, rfd_);

    int error = get_error(rfd_);
    if (error != 0)
    {
        WARN("connector[%d-%d]: connect failed, %s\n", 
            lfd_, rfd_,
            const_cast<const char*>(strerror(error)));
        event_->del(lfd_);
        event_->del(rfd_);
        close(lfd_);
        close(rfd_);
        delete this;
        return Event::ERR;
    }

    DEBUG("connector[%d-%d]: dup read & write fd\n", lfd_, rfd_);
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
        WARN("connector[%d-%d]: dup read & write fd failed\n", lfd_, rfd_);
        return Event::ERR;
    }

    DEBUG("connector[%d-%d]: init buffer and readwriter, add event[rw]\n",
        lfd_, rfd_);
    SharedBuffer rbuf = std::make_shared<ZBuffer>();
    SharedBuffer wbuf = std::make_shared<ZBuffer>();
    ReadWriter* rw_fwd = new ReadWriter(event_, lfd_, lfd2, rbuf, wbuf);
    ReadWriter* rw_rev = new ReadWriter(event_, rfd_, rfd2, wbuf, rbuf);
    rw_fwd->set_another(rw_rev);
    rw_rev->set_another(rw_fwd);
    event_->add(lfd_, EPOLLIN|EPOLLET, rw_fwd);
    //event_->add(lfd2, EPOLLOUT|EPOLLET|EPOLLONESHOT, rw_fwd);
    event_->mod(rfd_, EPOLLIN|EPOLLET, rw_rev);
    //event_->add(rfd2, EPOLLOUT|EPOLLET|EPOLLONESHOT, rw_rev);
    delete this;
    DEBUG("connector[%d-%d]: delete connector\n", lfd_, rfd_);
    return Event::OK;
}