#include <iostream>

#include "event.h"
#include "endpoint.h"
#include "listener.h"
#include "readwriter.h"
#include "utils.h"


int main(int argc, char **argv)
{
    auto event = std::make_shared<Event>();
    auto lsa = OwnedSA(to_sockaddr(20000));
    auto rsa = SharedSA(to_sockaddr("127.0.0.1",30000));
    auto lis = std::make_unique<Listener>(event,rsa,std::move(lsa));
    
    event->add(lis->inner_fd(), EPOLLIN|EPOLLET, lis.get());
    event->run();
}