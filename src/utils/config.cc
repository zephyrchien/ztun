#include "config.h"


Config::Config(const string l_addr, const int l_port,
    const string r_addr, const int r_port):
    local_addr(l_addr), local_port(l_port),
    remote_addr(r_addr), remote_port(r_port)
    { }

Config::~Config() { }