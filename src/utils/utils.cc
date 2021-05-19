#include "utils.h"


int set_reuseaddr(const int fd)
{
    int opt = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

int set_nonblocking(const int fd)
{
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_opt);
    return old_opt;
}

int get_error(const int fd)
{
    int error = 0;
    socklen_t len = sizeof(error);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
    return error;
}

int dup_with_opt(const int fd)
{
    int fd2 = dup(fd);
    if (fd2 < 0) return -1;
    int opt = fcntl(fd, F_GETFL);
    fcntl(fd2, F_SETFL, opt);
    return fd2;
}

sockaddr_in* to_sockaddr(const int port)
{
    sockaddr_in* sa = new sockaddr_in;
    memset(sa, 0, SALEN);
    sa->sin_family = AF_INET;
    sa->sin_port = htons(port);
    return sa;
}

sockaddr_in* to_sockaddr(const char* addr, const int port)
{
    sockaddr_in* sa = new sockaddr_in;
    memset(sa, 0, SALEN);
    sa->sin_family = AF_INET;
    sa->sin_port = htons(port);
    inet_pton(AF_INET, addr, &sa->sin_addr);
    return sa;
}

sockaddr_in* to_sockaddr(const string& addr, const int port)
{
    sockaddr_in* sa = new sockaddr_in;
    memset(sa, 0, SALEN);
    sa->sin_family = AF_INET;
    sa->sin_port = htons(port);
    inet_pton(AF_INET, addr.c_str(), &sa->sin_addr);
    return sa;
}

const string to_string(const sockaddr_in* sa)
{
    char addr[INET_ADDRSTRLEN];
    stringstream ss;
    ss << inet_ntop(AF_INET, &sa->sin_addr, addr, INET_ADDRSTRLEN)
        << ":" << sa->sin_port;
    return ss.str();
}

string& trim_space_left(string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char ch) {
        return !std::isspace(ch);
    }));
    return s;
}

string& trim_space_right(string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}

string& trim_space(string& s)
{
    trim_space_left(s);
    trim_space_right(s);
    return s;
}

string trim_space_copy(string s)
{
    trim_space_left(s);
    trim_space_right(s);
    return s;
}

int split_addr_port(const string& s, pair<string, int>& res)
{
    int pos = s.find_last_of(':');
    if (pos == std::string::npos) return -1;
    try {
        res.second = std::stoi(s.substr(pos + 1));
    } catch(...) {
        return -1;
    }
    if (pos == 0)
        res.first = "0.0.0.0";
    else
        res.first = s.substr(0, pos);
    return 0;
}

int split_key_value(const string& s, pair<string, string>& res)
{
    int pos = s.find('=');
    if (pos == std::string::npos) return -1;
    res.first = trim_space_copy(s.substr(0, pos));
    res.second = trim_space_copy(s.substr(pos + 1));
    return 0;
}

int read_config(Configs& configs, const string& file)
{
    ifstream fp(file);
    if (fp.bad()) return -1;
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
        // collect a pair of local & remote
        if (split_key_value(line, kv) < 0) return n;
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
            configs.emplace_back(Config(
                std::move(local.first), local.second,
                std::move(remote.first), remote.second
            ));
            with_local = with_remote = false;
        }
    }
    return 0;
}

int read_config(Configs& configs, const string& local_s, const string& remote_s)
{
    pair<string, int> local, remote;
    if (split_addr_port(local_s, local) < 0) return -1;
    if (split_addr_port(remote_s, remote) < 0) return -1;
    configs.emplace_back(Config(
        std::move(local.first), local.second,
        std::move(remote.first), remote.second
    ));
    return 0;
}