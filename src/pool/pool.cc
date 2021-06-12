#include "pool.h"


template<typename T> LinkList<T>* HEAD = nullptr;

template<typename T>
LinkList<T>* LinkList<T>::create(const int size)
{
    LinkList<T>* head = NEWT;
    //if (ISPIPE) init_pipe(head);
    LinkList<T>* last = head;
    for (int i = 0; i < size; i++)
    {
        LinkList<T> *next = NEWT;
        //if (ISPIPE) init_pipe(next);
        last->next = next;
        last = next;
    }
    last->next = nullptr;
    HEAD = head;
    return head;
}

template<typename T>
void LinkList<T>::destroy(LinkList<T>* head)
{
    LinkList<T> *ptr = head;
    LinkList<T> *px;
    while (ptr != nullptr)
    {
        px = ptr;
        ptr = ptr->next;
        //if (ISPIPE) close_pipe(px);
        std::free(px);
    }
}

template<typename T>
LinkList<T>* LinkList<T>::alloc()
{
    LinkList<T> *ptr = HEAD;
    if (ptr->next == nullptr)
    {
        LinkList<T> *next = NEWT;
        next->next = nullptr;
        //if (ISPIPE) init_pipe(next);
        ptr->next = next;
    }
    HEAD = ptr->next;
    return ptr;
}

template<typename T>
void LinkList<T>::collect(LinkList<T>* ptr)
{
    LinkList<T> *head = HEAD;
    HEAD = ptr;
    ptr->next = head;
}

/**
template<>
void LinkList<ZBuffer>::init_pipe(LinkList<ZBuffer>* ptr)
{
    int rw_pipe[2];
    int ret __attribute__((unused)) = pipe2(rw_pipe, O_NONBLOCK);
    ptr->elem.rfd = rw_pipe[0];
    ptr->elem.wfd = rw_pipe[1];
}

template<>
void LinkList<ZBuffer>::close_pipe(LinkList<ZBuffer>* ptr)
{
    close(ptr->elem.rfd);
    close(ptr->elem.wfd);
}
*/

Endpoint Pool::empty_ep = Endpoint(
    [](uint32_t){ return 0; }, 
    [] { return 0; }
);

std::unique_ptr<Pool> Pool::instance = nullptr;

Pool::Pool(LinkList<Timer>* t, LinkList<ZBuffer>* z,
        LinkList<Connector>* cc, LinkList<ReadWriter>* rw):
        timer_pool(t), pipe_pool(z),
        cc_pool(cc), rw_pool(rw) { }

Pool::~Pool()
{
    LinkList<Timer>::destroy(instance->timer_pool);
    LinkList<ZBuffer>::destroy(instance->pipe_pool);
    LinkList<Connector>::destroy(instance->cc_pool);
    LinkList<ReadWriter>::destroy(instance->rw_pool);
}

int Pool::init(const int size)
{
    auto t = LinkList<Timer>::create(size*TIMER_PREALLOC_RATIO);
    auto z = LinkList<ZBuffer>::create(size*BUFFER_PREALLOC_RATIO);
    auto cc = LinkList<Connector>::create(size*CC_PREALLOC_RATIO);
    auto rw = LinkList<ReadWriter>::create(size*RW_PREALLOC_RATIO);
    instance = std::unique_ptr<Pool>(new Pool(t, z, cc, rw));
    return 0;
}

template LinkList<Timer>* LinkList<Timer>::alloc();
template LinkList<ZBuffer>* LinkList<ZBuffer>::alloc();
template LinkList<Connector>* LinkList<Connector>::alloc();
template LinkList<ReadWriter>* LinkList<ReadWriter>::alloc();

template void LinkList<Timer>::collect(LinkList<Timer>*);
template void LinkList<ZBuffer>::collect(LinkList<ZBuffer>*);
template void LinkList<Connector>::collect(LinkList<Connector>*);
template void LinkList<ReadWriter>::collect(LinkList<ReadWriter>*);