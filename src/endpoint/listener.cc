#include "listener.h"


int Listener::timeout = DEFAULT_CONNECT_TIMEOUT;

Listener::Listener(Event* event, const addrinfo* hints,
    const sa_family_t family, const sockaddr_storage* ss):
    ev(event), fd_(socket(family, SOCK_STREAM, 0)),
    family_(family), hints_(hints)
{
    set_reuseaddr(fd_);
    set_nonblocking(fd_);
    auto sa = reinterpret_cast<const sockaddr*>(ss);
    INFO("bind and listen %s\n",
        to_string(family_, sa).c_str());
    if (bind(fd_, sa, sizeof(sockaddr_in6)) < 0)
    {
        close(fd_);
        throw std::runtime_error(strerror(errno));
    }
    listen(fd_, backlog);
}

Listener::~Listener()
{
    close(fd_);
    delete hints_;
}

int Listener::inner_fd() const
{
    return fd_;
}

int Listener::on_accept()
{
    DEBUG("listener[%d]: on accept\n", fd_);

    sockaddr_storage ss;
    auto sa = reinterpret_cast<sockaddr*>(&ss);
    socklen_t len = (family_ == AF_INET)?
        sizeof(sockaddr_in) : sizeof(sockaddr_in6);
    while (true)
    {
        int conn = accept4(fd_, sa, &len, SOCK_NONBLOCK);
        if (conn == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK
                && errno != ECONNABORTED && errno != EPROTO)
                WARN("accept error %s\n",
                const_cast<const char*>(strerror(errno)));
            return 0;
        }
        set_nodelay(conn);
        INFO("new connection from %s\n",
            to_string(family_, sa).c_str());

        int rfd = socket(AF_INET, SOCK_STREAM, 0);
        set_nonblocking(rfd);
        set_nodelay(rfd);
        Connector* c = new Connector(ev, rfd, conn);
        // bind callback func
        c->ep.callback = [=](uint32_t e) {
            return c->callback(e);
        };
        c->ep.on_timeout = [=]() {
            return c->on_timeout();
        };
        
        DEBUG("listener[%d]: add event[connect]\n", fd_);
        ev->add(rfd, EPOLLOUT|EPOLLET|EPOLLONESHOT, &c->ep);
        DEBUG("listener[%d]: set timeout[connect]\n", fd_);
        Timer* t = TimeWheel::instance()->add(timeout, &c->ep);
        c->timer = t;
        int ret = connect(rfd, hints_->ai_addr, hints_->ai_addrlen);

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
        WARN("connect failed immediately, %s\n",
            fd_, const_cast<const char*>(strerror(errno)));
        ev->del(rfd);
        delete c;
        close(conn);
        close(rfd);
    }
    return 0;
}
