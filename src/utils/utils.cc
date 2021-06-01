#include "utils.h"


int set_dualstack(const int fd)
{
    int opt = 0;
    return setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
}

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

sockaddr_storage* to_sockaddr(const int port)
{
    auto sa_in6 = new sockaddr_in6;
    memset(sa_in6, 0, sizeof(sockaddr_in6));
    sa_in6->sin6_family = AF_INET6;
    sa_in6->sin6_port = htons(port);
    return reinterpret_cast<sockaddr_storage*>(sa_in6);
}

sockaddr_storage*
to_sockaddr(sa_family_t& f, const char* addr, const int port)
{
    auto ss = new sockaddr_storage;
    memset(ss, 0, sizeof(sockaddr_storage));
    // try ipv4
    auto sa_in = reinterpret_cast<sockaddr_in*>(ss);
    if (inet_pton(AF_INET, addr, &sa_in->sin_addr) == 1)
    {
        sa_in->sin_port = htons(port);
        f = sa_in->sin_family = AF_INET;
        return ss;
    }
    // try ipv6
    auto sa_in6 = reinterpret_cast<sockaddr_in6*>(ss);
    if (inet_pton(AF_INET6, addr, &sa_in6->sin6_addr) == 1)
    {
        sa_in6->sin6_port = htons(port);
        f = sa_in6->sin6_family = AF_INET6;
        return ss;
    }
    delete ss;
    return nullptr;
}

sockaddr_storage* 
to_sockaddr(sa_family_t& f, const string& addr, const int port)
{
    return to_sockaddr(f, addr.c_str(), port);
}

const string to_string(const sockaddr_in* sa)
{
    char addr[INET_ADDRSTRLEN];
    stringstream ss;
    ss << inet_ntop(AF_INET, &sa->sin_addr, addr, INET_ADDRSTRLEN)
        << ":" << ntohs(sa->sin_port);
    return ss.str();
}

const string to_string(const sockaddr_in6* sa_in6)
{
    char addr[INET6_ADDRSTRLEN];
    stringstream ss;
    ss << inet_ntop(AF_INET6, &sa_in6->sin6_addr, addr, INET6_ADDRSTRLEN)
        << ":" << ntohs(sa_in6->sin6_port);
    return ss.str();
}

const string to_string(const sa_family_t f, const sockaddr *sa)
{
    string s;
    if (f == AF_INET)
        s = to_string(reinterpret_cast<const sockaddr_in*>(sa));
    else if (f == AF_INET6)
        s = to_string(reinterpret_cast<const sockaddr_in6*>(sa));
    return s;
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
    auto pos = s.find_last_of(':');
    if (pos == std::string::npos) return -1;
    try {
        res.second = std::stoi(s.substr(pos + 1));
    } catch(...) {
        return -1;
    }
    if (pos == 0)
        res.first = "::";
    else
        res.first = s.substr(0, pos);
    return 0;
}

int split_key_value(const string& s, pair<string, string>& res)
{
    auto pos = s.find_first_of('=');
    if (pos == std::string::npos) return -1;
    res.first = trim_space_copy(s.substr(0, pos));
    string second = trim_space_copy(s.substr(pos + 1));
    // trim comment start with #
    pos = second.find_first_of('#');
    if (pos == std::string::npos) 
        res.second = std::move(second);
    else
        res.second = second.substr(0, pos);
    return 0;
}

LOG_LEVEL to_level(const string& s)
{
    LOG_LEVEL level = LOG_LEVEL::INFO;
    if (s == "debug") level = LOG_LEVEL::DEBUG;
    else if (s == "info") level = LOG_LEVEL::INFO;
    else if (s == "warn") level = LOG_LEVEL::WARN;
    else if (s == "none") level = LOG_LEVEL::NONE;
    return level;
}