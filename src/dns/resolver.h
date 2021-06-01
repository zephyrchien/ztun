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

class Resolver : public Endpoint
{
    private:
        const int fd_;
        static int intv_;
        static int timeout_;
        static addrinfo* hints_;
        static OwnedResolver r_;

    private:
        explicit Resolver(Event*, const int);

    public:
        vector<Query> qs;

    public:
        ~Resolver() override;
        static OwnedResolver& instance();
        static int init(Event*);
        static void set_timeout(const int, const int);
        static int resolve_intv();
        static addrinfo* inner_hints();
        static int sync_lookup(Query*);

    public:
        int callback(uint32_t) override;
        int timeout() override;
        void async_lookup(Query*);
};

#endif