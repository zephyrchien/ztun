#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include "utils.h"


using std::string;
using std::vector;
using std::pair;
using std::ifstream;

struct EndpointConfig
{
    int local_port;
    int remote_port;
    string local_addr;
    string remote_addr;
    EndpointConfig(const int, const int, const string, const string);
    ~EndpointConfig();
};

struct Config
{
    // global config
    string log_level;
    string log_file;
    string timer_intv;
    string connect_timeout;
    string resolve_intv;
    string resolve_timeout;

    // endpoint config
    vector<EndpointConfig> ep_vec;

    // func
    int from_cmd(string&, string&);
    int from_file(string&);
    int parse_global_conf(pair<string, string>&);
};

#endif
