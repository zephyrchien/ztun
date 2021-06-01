#include "listener.h"


int Listener::timeout_ = DEFAULT_CONNECT_TIMEOUT;

Listener::Listener(Event* event, const addrinfo* hints,
    const sa_family_t family, const sockaddr_storage* ss):
    Endpoint(event), fd_(socket(family, SOCK_STREAM, 0)),
    family_(family), hints_(hints)
{
    set_reuseaddr(fd_);
    set_nonblocking(fd_);
    auto sa = reinterpret_cast<const sockaddr*>(ss);
    INFO("listener[%d]: bind and listen %s\n",
        fd_, to_string(family_, sa).c_str());
    if (bind(fd_, sa, sizeof(sockaddr_in6)) < 0)
    {
        close(fd_);
        throw std::runtime_error(strerror(errno));
    }
    listen(fd_, backlog_);
}

Listener::~Listener()
{
    close(fd_);
    delete hints_;
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
                WARN("listener[%d]: accept error %s\n",
                fd_, const_cast<const char*>(strerror(errno)));
            return Event::OK;
        }
        INFO("listener[%d]: new connection %s -> %s\n",
            fd_, to_string(family_, sa).c_str(),
            to_string(hints_->ai_family, hints_->ai_addr).c_str());

        int rfd = socket(AF_INET, SOCK_STREAM, 0);
        set_nonblocking(rfd);
        Connector* c = new Connector(event_, rfd, conn);

        DEBUG("listener[%d]: add event[connect]\n", fd_);
        event_->add(rfd, EPOLLOUT|EPOLLET|EPOLLONESHOT, c);
        DEBUG("listener[%d]: set timeout[connect]\n", fd_);
        Timer* t = TimeWheel::instance()->add(timeout_, c);
        c->set_timer(t);
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