#include <iostream>
#include <vector>
#include <stdexcept>
#include <unistd.h>
#include <signal.h>
#include "log/log.h"
#include "utils/utils.h"
#include "utils/config.h"
#include "event/event.h"
#include "timer/wheel.h"
#include "dns/query.h"
#include "dns/resolver.h"
#include "endpoint/listener.h"

using std::vector;
using std::exception;
const static string version = "v0.1.0";
const static string usage = 
    "usage: ztun [-v] "
    "[-c <config>] "
    "[-l <local_s>] [-r <remote_s>]";

int read_config(int argc, char **argv, Config* config)
{
    opterr = 0;
    int opt;
    string local_s, remote_s, config_file;
    while ((opt = getopt(argc, argv, "vl:r:c:")) != -1)
    {
        switch (opt)
        {
            case 'v':
                std::cout << "ztun version: " << version << std::endl;
                exit(0);
            case 'l':
                local_s = optarg;
                break;
            case 'r':
                remote_s = optarg;
                break;
            case 'c':
                config_file = optarg;
                break;
            default:
                return -1;
        }
    }

    int ret;
    if (!config_file.empty())
        ret = config->from_file(config_file);
    else if(!remote_s.empty() && !local_s.empty())
        ret = config->from_cmd(local_s, remote_s);
    else ret = -1;
    return ret;
}

int init_logger(Config* config)
{
    int ret;
    auto level = static_cast<Log::LEVEL>(to_level(config->log_level));
    if (!config->log_file.empty())
        ret = Log::init(level, config->log_file.c_str());
    else
        ret = Log::init(level, STDERR_FILENO);
    return ret;
}

int init_timer(Event* event, Config* config)
{
    int timer_intv = DEFAULT_TIMER_INTV,
        connect_timeout = DEFAULT_CONNECT_TIMEOUT,
        resolve_intv = DEFAULT_RESOLVE_INTV,
        resolve_timeout = DEFAULT_RESOLVE_TIMEOUT;
    try {
        if (!config->timer_intv.empty())
            timer_intv = std::stoi(config->timer_intv);
        if (!config->connect_timeout.empty())
            connect_timeout = std::stoi(config->connect_timeout);
        if (!config->resolve_intv.empty())
            resolve_intv = std::stoi(config->resolve_intv);
        if (!config->resolve_timeout.empty())
            resolve_timeout = std::stoi(config->resolve_timeout);
    } catch (...) {
        return -1;
    }
    TimeWheel::intv = timer_intv;
    Listener::timeout = connect_timeout;
    Resolver::intv = resolve_intv;
    Resolver::timeout = resolve_timeout;
    return TimeWheel::init(event);
}

int init_resolver(Event* event, Config* config)
{
    int ret = Resolver::init(event);
    if (ret < 0) return -1;
    Resolver::instance()->qs.reserve(config->ep_vec.size());
    for (auto it = Resolver::instance()->qs.begin();
        it != Resolver::instance()->qs.end();
        it++)
    {
        it->ep.on_timeout = [=] {
            return it->on_timeout();
        };
    }
    return 0;
}

int init_utils(Event* event, Config* config)
{
    // init logger
    if (init_logger(config) < 0)
    {
        std::cerr << "failed to init logger" << std::endl;
        return -1;
    }
    // init timer
    if (init_timer(event, config) < 0)
    {
        std::cerr << "failed to init timer" << std::endl;
        return -1;
    }
    // init resolver
    if (init_resolver(event, config) < 0)
    {
        std::cerr << "failed to init resolver" << std::endl;
        return -1;
    }
    return 0;
}

int init_endpoints(vector<Listener>& lis, Event* event, Config* config)
{
    for (const auto& c: config->ep_vec)
    {
        std::cerr << "init: " <<
            c.local_addr << ":" << c.local_port
            << " -> " <<
            c.remote_addr << ":" << c.remote_port
        << std::endl;
        // local
        sa_family_t f;
        auto raw_lsa = to_sockaddr(f, c.local_addr, c.local_port);
        if (raw_lsa == nullptr)
        {
            std::cerr << "invalid local addr, quit" << std::endl;
            delete raw_lsa;
            return -1;
        }
        // remote
        auto hints = new addrinfo;
        memset(hints, 0, sizeof(addrinfo));
        hints->ai_addr = reinterpret_cast<sockaddr*>(new sockaddr_storage);
        Resolver::instance()->qs.emplace_back(hints,
            c.remote_addr, std::to_string(c.remote_port));
        auto q = &*Resolver::instance()->qs.rbegin();
        if (Resolver::sync_lookup(q) < 0)
        {
            std::cerr << "invalid remote addr, quit" << std::endl;
            delete raw_lsa;
            return -1;
        }
        try {
            lis.emplace_back(event, hints, f, raw_lsa);
        } catch (exception& e) {
            std::cerr << e.what() << std::endl;
            delete raw_lsa;
            return -1;
        }
        delete raw_lsa;
    }
    TimeWheel::instance()->add(
        Resolver::intv, &Resolver::instance()->ep, true);
    return 0;
}

int main(int argc, char **argv)
{
    // ignore SIGPIPE when write to a closed socket
    signal(SIGPIPE, SIG_IGN);

    // load config from cmd or file
    int ret = 0;
    auto config = new Config();
    ret = read_config(argc, argv, config);
    if (ret < 0)
    {
        std::cerr << usage << std::endl;
        delete config;
        return 1;
    }else if (ret > 0)
    {
        std::cerr << "error in config file: line " << ret << std::endl;
        delete config;
        return 1;
    }

    // main event loop
    auto event = new Event();

    // init logger, timer, resolver
    if (init_utils(event, config) < 0)
    {
        delete config;
        delete event;
        return 1;
    }

    // init endpoints
    vector<Listener> lis;
    lis.reserve(config->ep_vec.size());
    if (init_endpoints(lis, event, config) < 0)
    {
        delete config;
        delete event;
        return 1;
    }
    delete config;

    // start process
    for (auto it = lis.begin(); it != lis.end(); it++)
    {
        it->ep.callback = [=](uint32_t ev) {
            return it->callback(ev);
        };
        event->add(it->inner_fd(), EPOLLIN|EPOLLET, &it->ep);
    }
    event->run();
    delete event;
}