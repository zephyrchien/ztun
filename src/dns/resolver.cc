#include "resolver.h"


int Resolver::intv = DEFAULT_RESOLVE_INTV;
int Resolver::timeout = DEFAULT_RESOLVE_TIMEOUT;
addrinfo* Resolver::query_hints = nullptr;
OwnedResolver Resolver::r_ = OwnedResolver(nullptr);

Resolver::Resolver(Event* event, const int fd):
    ev(event), fd_(fd){ }

Resolver::~Resolver()
{
    close(fd_);
    delete query_hints;
}


OwnedResolver& Resolver::instance()
{
    return r_;
}

int Resolver::init(Event* event)
{
    // block SIGRTMIN
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGRTMIN);
    sigprocmask(SIG_BLOCK, &mask, nullptr);
    int fd = signalfd(-1, &mask, SFD_NONBLOCK);
    if (fd < 0) return -1;
    r_ = OwnedResolver(new Resolver(event, fd));
    // shared query hints
    addrinfo* hints = new addrinfo;
    memset(hints, 0, sizeof(addrinfo));
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM;
    query_hints = hints;
    // bind callback func
    r_->ep.callback = [&](uint32_t e) {
        return r_->callback(e);
    };
    r_->ep.on_timeout = [&] {
        return r_->on_timeout();
    };
    event->add(fd, EPOLLIN|EPOLLET, &r_->ep);
    return 0;
}

int Resolver::callback(uint32_t event)
{
    Query *q;
    signalfd_siginfo info;
    while (read(fd_, &info, sizeof(info)) > 0)
    {
        q = reinterpret_cast<Query*>(info.ssi_ptr);
        q->callback(0);
    }
    return Event::OK;
}

int Resolver::on_timeout()
{
    for (auto& q:qs)
    {
        async_lookup(&q);
    }
    return 0;
}

void Resolver::async_lookup(Query *q)
{
    DEBUG("resolver: lookup %s\n", q->name.c_str());
    sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = q;
    int ret = getaddrinfo_a(GAI_NOWAIT, &q->data, 1, &sev);
    if (ret != 0)
    {
        WARN("resolver: lookup %s failed, %s\n", q->name.c_str(),
            gai_strerror(ret));
        return;
    }
    Timer *t = TimeWheel::instance()->add(
        DEFAULT_RESOLVE_TIMEOUT, &q->ep);
    q->timer = t;
}

int Resolver::sync_lookup(Query *q)
{
    int ret = getaddrinfo_a(GAI_WAIT, &q->data, 1, nullptr);
    if (ret != 0 || gai_error(q->data) != 0)
    {
        freeaddrinfo(q->data->ar_result);
        q->data->ar_result = nullptr;
        return -1;
    }
    q->hints->ai_family = q->data->ar_result->ai_family;
    q->hints->ai_addrlen = q->data->ar_result->ai_addrlen;
    int n = (q->hints->ai_family == AF_INET)?
        sizeof(sockaddr_in) : sizeof(sockaddr_in6);
    auto src = reinterpret_cast<char*>(q->data->ar_result->ai_addr);
    auto dst = reinterpret_cast<char*>(q->hints->ai_addr);
    std::copy_n(src, n, dst);
    freeaddrinfo(q->data->ar_result);
    q->data->ar_result = nullptr;
    return 0;
}