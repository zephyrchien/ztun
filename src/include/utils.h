#ifndef UTILS_H_
#define UTILS_H_

#include <memory>
#include <string>
#include <sstream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>

using std::string;
using std::stringstream;

const static int SALEN = sizeof(sockaddr_in);
class Event;
class Buffer;
class Endpoint;
class Listener;
class Reader;
class Writer;
typedef std::shared_ptr<Event> SharedEvent;
typedef std::shared_ptr<Buffer> SharedBuffer;
typedef std::unique_ptr<Buffer> OwnedBuffer;
typedef std::shared_ptr<sockaddr_in> SharedSA;
typedef std::unique_ptr<sockaddr_in> OwnedSA;

int set_reuseaddr(const int);
int set_nonblocking(const int);
int get_error(const int);
int dup_with_opt(const int);
sockaddr_in* to_sockaddr(const int);
sockaddr_in* to_sockaddr(const char*, const int);
const string to_string(const sockaddr_in*);

#endif
