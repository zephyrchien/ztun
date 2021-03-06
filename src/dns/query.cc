#include "query.h"


Query::Query(addrinfo* h, const string& n, const string& s):
    name(n), service(s), hints(h), timer(nullptr)
{
    data = new gaicb;
    data->ar_name = name.c_str();
    data->ar_service = service.c_str();
    data->ar_request = Resolver::query_hints;
    data->ar_result = nullptr;
}

Query::~Query()
{
    if (data->ar_result != nullptr) 
        freeaddrinfo(data->ar_result);
    delete data;
    delete hints->ai_addr;
    delete hints;
}

int Query::callback(uint32_t event)
{
    TimeWheel::instance()->del(timer);
    timer = nullptr;
    int ret = gai_error(data);
    if (ret != 0)
    {
        WARN("resolve %s failed, %s\n", name.c_str(),
            gai_strerror(ret));
        data->ar_result = nullptr;
        return -1;
    }
    // reload sockaddr
    hints->ai_family = data->ar_result->ai_family;
    hints->ai_addrlen = data->ar_result->ai_addrlen;
    int n = (hints->ai_family == AF_INET)?
        sizeof(sockaddr_in) : sizeof(sockaddr_in6);
    auto src = reinterpret_cast<char*>(data->ar_result->ai_addr);
    auto dst = reinterpret_cast<char*>(hints->ai_addr);
    std::copy_n(src, n, dst);
    INFO("resolve %s => %s\n", name.c_str(),
        to_string(hints->ai_family, hints->ai_addr).c_str());
    freeaddrinfo(data->ar_result);
    data->ar_result = nullptr;
    return 0;
}

int Query::on_timeout()
{
    WARN("resolve %s timeout\n", name.c_str());
    timer = nullptr;
    gai_cancel(data);
    data->ar_result = nullptr;
    return 0;
}