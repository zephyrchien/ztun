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

const string to_string(const sockaddr_in* sa)
{
    char addr[INET_ADDRSTRLEN];
    stringstream ss;
    ss << inet_ntop(AF_INET, &sa->sin_addr, addr, INET_ADDRSTRLEN)
        << ":" << sa->sin_port;
    return ss.str();
}