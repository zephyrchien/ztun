#ifndef RESOLVER_H_
#define RESOLVER_H_

#include <string>
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

#define DEFAULT_RESOLVE_TIMEOUT 2000

using std::string;

class Resolver : public Endpoint
{
    private:
        const int fd_;
        static addrinfo* hints_;
        static OwnedResolver r_;

    private:
        explicit Resolver(const SharedEvent, const int);

    public:
        ~Resolver() override;
        static OwnedResolver& instance();
        static int init(const SharedEvent);
        static addrinfo* inner_hints();

    public:
        int callback(uint32_t) override;
        int timeout() override;
        int sync_lookup(Query*);
        void async_lookup(Query*);

};

#endif