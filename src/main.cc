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
#include "endpoint/endpoint.h"
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

int init_timer(Config* config, Event* event)
{
    int intv, timeout;
    try {
        intv = std::stoi(config->timer_intv);
        timeout = std::stoi(config->connect_timeout);
    } catch (...) {
        intv = timeout = -1;
        std::cerr << 
            "warning: bad intv|timeout, use default value"
        << std::endl;
    }
    if (timeout > 0) Listener::set_timeout(timeout);
    if (intv > 0) TimeWheel::set_intv(intv);
    return TimeWheel::init(event);
}

int main(int argc, char **argv)
{
    // ignore SIGPIPE when write to a closed socket
    signal(SIGPIPE, SIG_IGN);
    int ret = 0;
    auto config = new Config();

    // load config from cmd or file
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

    // init logger
    if ((ret = init_logger(config)) < 0)
    {
        std::cerr << "failed to init logger" << std::endl;
        delete config;
        return 1;
    }
    // init timer
    if ((ret = init_timer(config, event)) < 0)
    {
        std::cerr << "failed to init timer" << std::endl;
        delete config;
        return 1;
    }
    // init endpoints
    vector<Listener> listeners;
    listeners.reserve((config->ep_vec.size()));
    for (const auto& c: config->ep_vec)
    {
        std::cerr << "init " <<
            c.local_addr << ":" << c.local_port
            << " -> " <<
            c.remote_addr << ":" << c.remote_port
        << std::endl;
        auto raw_lsa = to_sockaddr(c.local_addr, c.local_port);
        auto raw_rsa = to_sockaddr(c.remote_addr, c.remote_port);
        if (raw_lsa == nullptr || raw_rsa == nullptr)
        {
            std::cerr << "invalid addr, quit" << std::endl;
            delete raw_lsa;
            delete raw_rsa;
            delete config;
            return 1;
        }
        try {
            listeners.emplace_back(event, raw_rsa, raw_lsa);
        } catch (exception& e) {
            std::cerr << e.what() << std::endl;
            delete raw_lsa;
            delete config;
            return 1;
        }
        delete raw_lsa;
    }
    delete config;
    // start process
    for (const auto& lis: listeners)
    {
        event->add(lis.inner_fd(), EPOLLIN|EPOLLET, &lis);
    }
    event->run();
}