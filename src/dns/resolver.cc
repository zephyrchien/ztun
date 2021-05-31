#include "resolver.h"


addrinfo* Resolver::hints_ = nullptr;
OwnedResolver Resolver::r_ = OwnedResolver(nullptr);

Resolver::Resolver(const SharedEvent event, const int fd):
    Endpoint(event), fd_(fd) { }

Resolver::~Resolver()
{
    close(fd_);
    delete hints_;
}

addrinfo* Resolver::inner_hints()
{
    return hints_;
}

int Resolver::init(const SharedEvent event)
{
    // block SIGRTMIN
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGRTMIN);
    sigprocmask(SIG_BLOCK, &mask, nullptr);
    int fd = signalfd(-1, nullptr, SFD_NONBLOCK);
    if (fd < 0) return -1;
    r_ = OwnedResolver(new Resolver(event, fd));

    addrinfo* hints = new addrinfo;
    memset(hints, 0, sizeof(addrinfo));
    hints->ai_family = AF_INET;
    hints->ai_socktype = SOCK_STREAM;
    hints_ = hints;
    
    event->add(fd, EPOLLIN|EPOLLET, r_.get());
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
}

int Resolver::timeout()
{
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
        DEFAULT_RESOLVE_TIMEOUT, q);
    q->timer = t;
}

int Resolver::sync_lookup(Query *q)
{
    int ret = getaddrinfo_a(GAI_WAIT, &q->data, 1, nullptr);
    if (ret != 0 || gai_error(q->data) != 0) return -1;
    return 0;
}