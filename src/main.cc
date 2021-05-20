#include <iostream>

#include <iostream>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include "event.h"
#include "endpoint.h"
#include "listener.h"
#include "readwriter.h"
#include "utils.h"
#include "config.h"

using std::vector;
const static string version = "v0.1.0";
const static string usage = 
    "usage: ztun [-v] "
    "[-c <config>] "
    "[-l <local_s>] [-r <remote_s>]";

int main(int argc, char **argv)
{
    // ignore SIGPIPE when write to a closed socket
    signal(SIGPIPE, SIG_IGN);

    opterr = 0;
    int opt;
    string local_s, remote_s, config_file;
    while ((opt = getopt(argc, argv, "vl:r:c:")) != -1)
    {
        switch (opt)
        {
            case 'v':
                std::cout << "ztun version: " << version << std::endl;
                return 0;
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
                std::cerr << usage << std::endl;
                return 1;
        }
    }
    vector<Config> configs;
    int ret;
    if (!config_file.empty())
        ret = read_config(configs, config_file);
    else if(!remote_s.empty() && !local_s.empty())
        ret = read_config(configs, local_s, remote_s);
    else ret = -1;
    if (ret < 0)
    {
        std::cerr << "unexpected error" << std::endl;
        std::cerr << usage << std::endl;
        return 1;
    }else if (ret > 0)
    {
        std::cerr << "config: line " << ret << std::endl;
        return 1;
    }

    auto event = std::make_shared<Event>();
    vector<Listener> listeners;
    listeners.reserve((configs.size()));
    for (const auto& c: configs)
    {
        std::cout << 
            c.local_addr << ":" << c.local_port
            << " -> " <<
            c.remote_addr << ":" << c.remote_port
        << std::endl;
        auto lsa = OwnedSA(to_sockaddr(c.local_addr, c.local_port));
        auto rsa = SharedSA(to_sockaddr(c.remote_addr, c.remote_port));
        listeners.emplace_back(event, rsa, std::move(lsa));
    }
    for (const auto& lis: listeners)
    {
        event->add(lis.inner_fd(), EPOLLIN|EPOLLET, &lis);
    }
    configs.clear();
    configs.shrink_to_fit();
    event->run();
}