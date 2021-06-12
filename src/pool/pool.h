#ifndef POOL_H_
#define POOL_H_

#include <memory>
#include <type_traits>
#include <memory.h>
#include <unistd.h>
#include "timer/timer.h"
#include "buffer/zbuffer.h"
#include "endpoint/endpoint.h"
#include "endpoint/connector.h"
#include "endpoint/readwriter.h"

#define PREALLOC_SIZE 30
#define TIMER_PREALLOC_RATIO 2
#define BUFFER_PREALLOC_RATIO 2
#define CC_PREALLOC_RATIO 1
#define RW_PREALLOC_RATIO 2
#define NEWT static_cast<LinkList<T>*>\
    (std::malloc(sizeof(LinkList<T>)))
#define HEAD LinkList<T>::head
#define ISPIPE std::is_same<T, ZBuffer>::value

template<typename T>
struct LinkList
{
    T elem;
    LinkList<T>* next;
    static LinkList<T> *head;

    static LinkList<T>* create(const int);
    static void destroy(LinkList<T>*);
    static LinkList<T>* alloc();
    static void collect(LinkList<T>*);
    //static void init_pipe(LinkList<T>*);
    //static void close_pipe(LinkList<T>*);
};

struct Pool
{
    LinkList<Timer> *timer_pool;
    LinkList<ZBuffer> *pipe_pool;
    LinkList<Connector> *cc_pool;
    LinkList<ReadWriter> *rw_pool;
    static Endpoint empty_ep;
    static std::unique_ptr<Pool> instance;

    explicit Pool(LinkList<Timer>*, LinkList<ZBuffer>*,
        LinkList<Connector>*, LinkList<ReadWriter>*);
    ~Pool();
    static int init(const int);
};

#endif
