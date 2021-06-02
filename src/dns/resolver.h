#ifndef RESOLVER_H_
#define RESOLVER_H_

#include <string>
#include <vector>
#include <memory>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <sys/signalfd.h>
#include "utils/utils.h"
#include "log/log.h"
#include "timer/wheel.h"
#include "endpoint/endpoint.h"
#include "dns/query.h"

#define DEFAULT_RESOLVE_INTV 60000
#define DEFAULT_RESOLVE_TIMEOUT 2000


using std::string;
using std::vector;

class Resolver
{
    public:
        Event* ev;
        Endpoint ep;
        vector<Query> qs;

    public:
        static int intv;
        static int timeout;
        static addrinfo* query_hints;

    private:
        const int fd_;

    private:
        static OwnedResolver r_;
        explicit Resolver(Event*, const int);

    public:
        ~Resolver();
        static OwnedResolver& instance();
        static int init(Event*);
        
    public:
        int on_timeout();
        int callback(uint32_t);
        void async_lookup(Query*);
        static int sync_lookup(Query*);
};

#endif