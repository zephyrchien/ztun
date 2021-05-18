#include "listener.h"


Listener::Listener(const SharedEvent event, const SharedSA rsa, const int lfd):
    Endpoint(event), rsa_(rsa), lfd_(lfd) { }

Listener::Listener(const SharedEvent event, const SharedSA rsa, const OwnedSA lsa):
    Endpoint(event), rsa_(rsa), lfd_(socket(AF_INET, SOCK_STREAM, 0))
{
    set_reuseaddr(lfd_);
    set_nonblocking(lfd_);
    bind(lfd_, (sockaddr *)lsa.get(), SALEN);
    listen(lfd_, backlog_);
}

Listener::~Listener() { }

int Listener::inner_fd() const
{
    return lfd_;
}

int Listener::on_accept()
{
    sockaddr_in sa;
    socklen_t len = SALEN;
    int conn = accept(lfd_, (sockaddr *)&sa, &len);
    if (conn == -1) return Event::OK;
    set_nonblocking(conn);
    int rfd = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblocking(rfd);
    Connector* c = new Connector(event_, rfd, conn);
    event_->add(rfd, EPOLLOUT|EPOLLET|EPOLLONESHOT, c);
    int ret = connect(rfd, (sockaddr *)rsa_.get(), SALEN);
    if (ret == 0)
    {
        c->on_connect();
        return Event::OK;
    }
    if (errno == EINPROGRESS)
    {
        errno = 0;
        return Event::OK;
    }
    errno = 0;
    event_->del(rfd);
    delete c;
    close(conn);
    close(rfd);
    return Event::ERR;
}

int Listener::callback(uint32_t events)
{
    if (events & EPOLLIN) return on_accept();
    return Event::ERR;
}