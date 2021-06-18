#ifndef UTILS_H_
#define UTILS_H_

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <utility>
#include <algorithm> 
#include <functional> 
#include <locale>
#include <cctype>
#include <cstdint>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>
#include "config.h"

using std::string;
using std::stringstream;
using std::vector;
using std::pair;
using std::unordered_set;
using std::shared_ptr;
using std::unique_ptr;

enum LOG_LEVEL { NONE, WARN, INFO, DEBUG };
enum LOG_BUF_LINE { LNONE, LWARN = 2, LINFO = 5, LDEBUG = 10};

struct ZBuffer;
struct RingBuffer;
struct Timer;
struct Endpoint;
struct Config;
class Event;
class Log;
class TimeWheel;
class Listener;
class Connector;
class ReadWriter;
class Query;
class Resolver;

typedef unique_ptr<Log> OwnedLogger;
typedef unique_ptr<TimeWheel> OwnedTimer;
typedef unique_ptr<Resolver> OwnedResolver;
typedef unique_ptr<RingBuffer> OwnedRingBuffer;

string& trim_space_left(string&);
string& trim_space_right(string&);
string& trim_space(string&);
string trim_space_copy(string);

int split_addr_port(const string&, pair<string, int>&);
int split_key_value(const string&, pair<string, string>&);

int set_dualstack(const int);
int set_reuseaddr(const int);
int set_nodelay(const int);
int set_nonblocking(const int);
int get_error(const int);
int dup_with_opt(const int);

sockaddr_storage* to_sockaddr(const int);
sockaddr_storage* to_sockaddr(sa_family_t&, const char*, const int);
sockaddr_storage* to_sockaddr(sa_family_t&, const string&, const int);
const string to_string(const sockaddr_in*);
const string to_string(const sockaddr_in6*);
const string to_string(sa_family_t, const sockaddr*);
LOG_LEVEL to_level(const string&);

#endif
