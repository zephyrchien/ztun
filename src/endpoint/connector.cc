#include "connector.h"


Connector::Connector(Event* event, const int rfd, const int lfd):
    ev(event), lfd_(lfd), rfd_(rfd) { }

Connector::~Connector() { }

void* Connector::operator new(std::size_t size)
{
    auto ptr = LinkList<Connector>::alloc();
    return ptr;
}

void Connector::operator delete(void *ptr)
{
    LinkList<Connector>::collect(
        static_cast<LinkList<Connector>*>(ptr)
    );
}

int Connector::on_timeout()
{
    WARN("connect timeout\n", lfd_, rfd_);
    ev->del(rfd_);
    close(lfd_);
    close(rfd_);
    delete this;
    return -1;
}

int Connector::on_connect()
{
    DEBUG("connector[%d-%d]: on connect\n", lfd_, rfd_);
    TimeWheel::instance()->del(timer);
    timer = nullptr;
    
    int error = get_error(rfd_);
    if (error != 0)
    {
        WARN("connect failed, %s\n", 
            lfd_, rfd_,
            const_cast<const char*>(strerror(error)));
        ev->del(rfd_);
        close(lfd_);
        close(rfd_);
        delete this;
        return -1;
    }

    DEBUG("connector[%d-%d]: dup read & write fd\n", lfd_, rfd_);
    int lfd2 = dup_with_opt(lfd_);
    int rfd2 = dup_with_opt(rfd_);
    if (lfd2 < 0 || rfd2 < 0)
    {
        ev->del(rfd_);
        close(lfd_);
        close(rfd_);
        close(lfd2);
        close(rfd2);
        delete this;
        WARN("dup read & write fd failed\n", lfd_, rfd_);
        return -1;
    }

    DEBUG("connector[%d-%d]: init buffer and readwriter, add event[rw]\n",
        lfd_, rfd_);
    ZBuffer* rbuf = new ZBuffer();
    ZBuffer* wbuf = new ZBuffer();
    rbuf->ref = wbuf->ref = 2;
    ReadWriter* rw_fwd = new ReadWriter(ev, lfd_, lfd2, rbuf, wbuf);
    ReadWriter* rw_rev = new ReadWriter(ev, rfd_, rfd2, wbuf, rbuf);
    rw_fwd->another = rw_rev; rw_rev->another = rw_fwd;
    // bind callback func
    rw_fwd->ep.callback = [=](uint32_t e) {
        return rw_fwd->callback(e);
    };
    rw_rev->ep.callback = [=](uint32_t e) {
        return rw_rev->callback(e);
    };
    ev->add(lfd_, EPOLLIN|EPOLLET, &rw_fwd->ep);
    ev->mod(rfd_, EPOLLIN|EPOLLET, &rw_rev->ep);
    delete this;
    DEBUG("connector[%d-%d]: delete connector\n", lfd_, rfd_);
    return 0;
}