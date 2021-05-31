#include "query.h"


Query::Query(const SharedEvent e, SharedSA r,
    const string& n, const string& s):
    Endpoint(e), name(n), service(s),
    rsa(r), timer(nullptr)
{
    data = new gaicb;
    data->ar_name = name.c_str();
    data->ar_service = service.c_str();
    data->ar_request = Resolver::inner_hints();
    data->ar_result = nullptr;
}

Query::~Query()
{
    freeaddrinfo(data->ar_result);
    delete data;
}

int Query::callback(uint32_t event)
{
    TimeWheel::instance()->del(timer);
    timer = nullptr;
    int ret = gai_error(data);
    if (ret != 0)
    {
        WARN("resolver: resolve %s failed, %s\n", name.c_str(),
            gai_strerror(ret));
        return -1;
    }
    INFO("resolver: resolve %s\n", name.c_str());
    // reload sockaddr
    //rsa_->sin_addr = data->ar_request->ai_addr->sa_data;
    freeaddrinfo(data->ar_result);
    data->ar_result = nullptr;
    return 0;
}

int Query::timeout()
{
    WARN("resolver: lookup %s timeout\n", name.c_str());
    timer = nullptr;
    gai_cancel(data);
}