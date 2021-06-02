#ifndef ENDPOINT_H_
#define ENDPOINT_H_

#include <memory>
#include <array>
#include <functional>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "utils/utils.h"
#include "event/event.h"


using std::function;

struct Endpoint
{
    Endpoint() { };
    ~Endpoint() { };
    function<int(uint32_t)> callback;
    function<int()> on_timeout;
};

#endif