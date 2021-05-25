#include "listener.h"


Listener::Listener(const SharedEvent event, const SharedSA rsa, const int fd):
    Endpoint(event), fd_(fd), rsa_(rsa) { }

Listener::Listener(const SharedEvent event, const SharedSA rsa, const OwnedSA lsa):
    Endpoint(event), fd_(socket(AF_INET, SOCK_STREAM, 0)), rsa_(rsa)
{
    set_reuseaddr(fd_);
    set_nonblocking(fd_);

    DEBUG("listener: bind and listen %s\n", to_string(lsa.get()).c_str());
    if (bind(fd_, (sockaddr *)lsa.get(), SALEN) < 0)
    {
        close(fd_);
        int e = errno;
        errno = 0;
        throw std::runtime_error(strerror(e));
    }
    listen(fd_, backlog_);
}

Listener::~Listener()
{
    close(fd_);
}

int Listener::inner_fd() const
{
    return fd_;
}

int Listener::on_accept()
{
    DEBUG("listener: on accept\n");

    sockaddr_in sa;
    socklen_t len = SALEN;
    int conn = accept(fd_, (sockaddr *)&sa, &len);
    if (conn == -1)
    {
        WARN("listener: accept error\n");
        return Event::OK;
    }
    INFO("listener: accept from %s\n",to_string(&sa).c_str());

    set_nonblocking(conn);
    int rfd = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblocking(rfd);
    Connector* c = new Connector(event_, rfd, conn);

    DEBUG("listener: add event[connect]\n");
    event_->add(rfd, EPOLLOUT|EPOLLET|EPOLLONESHOT, c);
    int ret = connect(rfd, (sockaddr *)rsa_.get(), SALEN);
    INFO("listener: connect to %s\n",to_string(rsa_.get()).c_str());

    if (ret == 0)
    {
        DEBUG("listener: connect succeed immediately\n");
        return Event::OK;
    }
    if (errno == EINPROGRESS)
    {
        DEBUG("listener: connect in progress\n");
        errno = 0;
        return Event::OK;
    }
    WARN("listener: connect failed immediately, %s\n", 
        const_cast<const char*>(strerror(errno)));
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