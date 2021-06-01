#ifndef QUERY_H_
#define QUERY_H_

#include <string>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include "utils/utils.h"
#include "log/log.h"
#include "timer/wheel.h"
#include "endpoint/endpoint.h"
#include "dns/resolver.h"


using std::string;

class Query : public Endpoint
{
    public:
        const string name;
        const string service;
        gaicb *data;
        addrinfo *hints;
        Timer* timer;

    public:
        explicit Query(Event*, addrinfo* hints,
            const string&, const string&);
        ~Query() override;

    public:
        int callback(uint32_t) override;
        int timeout() override;
};

#endif