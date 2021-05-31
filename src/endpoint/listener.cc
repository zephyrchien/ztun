#include "listener.h"


int Listener::timeout_ = DEFAULT_CONNECT_TIMEOUT;

Listener::Listener(Event* event, const sockaddr_in* rsa,
    const sockaddr_in* lsa):
    Endpoint(event), fd_(socket(AF_INET, SOCK_STREAM, 0)), rsa_(rsa)
{
    set_reuseaddr(fd_);
    set_nonblocking(fd_);

    INFO("listener[%d]: bind and listen %s\n", fd_, to_string(lsa).c_str());
    if (bind(fd_, reinterpret_cast<const sockaddr*>(lsa), SALEN) < 0)
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

void Listener::set_timeout(const int timeout)
{
    timeout_ = timeout;
}

int Listener::inner_fd() const
{
    return fd_;
}

int Listener::on_accept()
{
    DEBUG("listener[%d]: on accept\n", fd_);

    sockaddr_in sa;
    socklen_t len = SALEN;
    while (true)
    {
        int conn = accept4(fd_,
            reinterpret_cast<sockaddr*>(&sa),
            &len, SOCK_NONBLOCK);
        if (conn == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK
                && errno != ECONNABORTED && errno != EPROTO)
                WARN("listener[%d]: accept error %s\n",
                fd_, const_cast<const char*>(strerror(errno)));
            return Event::OK;
        }
        INFO("listener[%d]: new connection %s -> %s\n",
            fd_, to_string(&sa).c_str(),
            to_string(rsa_).c_str());

        int rfd = socket(AF_INET, SOCK_STREAM, 0);
        set_nonblocking(rfd);
        Connector* c = new Connector(event_, rfd, conn);

        DEBUG("listener[%d]: add event[connect]\n", fd_);
        event_->add(rfd, EPOLLOUT|EPOLLET|EPOLLONESHOT, c);
        DEBUG("listener[%d]: set timeout[connect]\n", fd_);
        Timer* t = TimeWheel::instance()->add(timeout_, c);
        c->set_timer(t);
        int ret = connect(rfd,
            reinterpret_cast<const sockaddr*>(rsa_), SALEN);

        if (ret == 0)
        {
            DEBUG("listener[%d]: connect succeed immediately\n", fd_);
            continue;
        }
        if (errno == EINPROGRESS)
        {
            DEBUG("listener[%d]: connect in progress\n", fd_);
            continue;
        }
        WARN("listener[%d]: connect failed immediately, %s\n",
            fd_, const_cast<const char*>(strerror(errno)));
        event_->del(rfd);
        delete c;
        close(conn);
        close(rfd);
    }
    return Event::OK;
}

int Listener::callback(uint32_t events)
{
    if (events & EPOLLIN) return on_accept();
    return Event::ERR;
}

int Listener::timeout()
{
    return 0;
}