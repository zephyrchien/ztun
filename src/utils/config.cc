#include "config.h"


Config::Config(const string& l_addr, const int l_port,
    const string& r_addr, const int r_port):
    local_port(l_port), remote_port(r_port),
    local_addr(l_addr),remote_addr(r_addr)
    { }

Config::~Config() { }