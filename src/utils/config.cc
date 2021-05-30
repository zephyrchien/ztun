#include "config.h"


EndpointConfig::EndpointConfig(const int lp, const int rp,
    const string la, const string ra):
    local_port(lp), remote_port(rp),
    local_addr(la), remote_addr(ra) { }

EndpointConfig::~EndpointConfig() { }

int Config::from_cmd(string& local_s, string& remote_s)
{
    pair<string, int> local, remote;
    if (split_addr_port(local_s, local) < 0) return -1;
    if (split_addr_port(remote_s, remote) < 0) return -1;
    ep_vec.emplace_back(local.second, remote.second,
        std::move(local.first), std::move(remote.first));
    return 0;
}

int Config::from_file(string& file)
{
    ifstream fp(file);
    if (!fp.good()) return -1;
    int n = 0;
    bool with_local = false, with_remote = false;
    string line;
    pair<string, string> kv;
    pair<string, int> local, remote;
    while(!fp.eof())
    {
        ++n;
        std::getline(fp, line);
        trim_space(line);
        if (line.empty() || line.at(0) == '#') continue;
        
        if (split_key_value(line, kv) < 0) return n;

        if (this->parse_global_conf(kv) == 0) continue;

        // try to collect a pair of local & remote
        if (!with_local && !with_remote)
        {
            if (kv.first == "local" && split_addr_port(kv.second,local) != -1)
                with_local = true;
            else if(kv.first == "remote" && split_addr_port(kv.second,remote) != -1)
                with_remote = true;
            else
                return n;
        } else if (with_local && !with_remote)
        {
            if (kv.first != "remote") return n;
            if (split_addr_port(kv.second, remote) < 0) return n;
            with_remote = true;
        } else if (!with_local && with_remote)
        {
            if (kv.first != "local") return n;
            if (split_addr_port(kv.second, local) < 0) return n;
            with_local = true;
        }
        // consume this pair
        if (with_local && with_remote)
        {
            ep_vec.emplace_back(local.second, remote.second,
                std::move(local.first), std::move(remote.first));
            with_local = with_remote = false;
        }
    }
    return 0;
}

int Config::parse_global_conf(pair<string, string>& kv)
{
    if(kv.first == "log_level")
        this->log_level = kv.second;
    else if (kv.first == "log_file")
        this->log_file = kv.second;
    else if (kv.first == "timer_intv")
        this->timer_intv = kv.second;
    else if (kv.first == "connect_timeout")
        this->connect_timeout = kv.second;
    else return -1;
    return 0;
}