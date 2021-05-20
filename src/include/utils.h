#ifndef UTILS_H_
#define UTILS_H_

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <utility>
#include <fstream>
#include <algorithm> 
#include <functional> 
#include <locale>
#include <cctype>
#include <cstdint>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>
#include "config.h"

using std::string;
using std::stringstream;
using std::vector;
using std::pair;
using std::unordered_set;
using std::ifstream;


const static int SALEN = sizeof(sockaddr_in);
class Event;
class Buffer;
class RingBuffer;
class Config;
class Endpoint;
class Listener;
class ReadWriter;
typedef std::shared_ptr<Event> SharedEvent;
typedef std::shared_ptr<Buffer> SharedBuffer;
typedef std::unique_ptr<Buffer> OwnedBuffer;
typedef std::shared_ptr<sockaddr_in> SharedSA;
typedef std::unique_ptr<sockaddr_in> OwnedSA;
typedef std::vector<Config> Configs;
typedef std::vector<Listener> Listeners;
typedef std::unordered_set<uintptr_t> PtrSet;

string& trim_space_left(string&);
string& trim_space_right(string&);
string& trim_space(string&);
string trim_space_copy(string);

int read_config(Configs&, const string&);
int read_config(Configs&, const string&, const string&);
int split_addr_port(const string&, pair<string, int>&);
int split_key_value(const string&, pair<string, string>&);

int set_reuseaddr(const int);
int set_nonblocking(const int);
int get_error(const int);
int dup_with_opt(const int);

sockaddr_in* to_sockaddr(const int);
sockaddr_in* to_sockaddr(const char*, const int);
sockaddr_in* to_sockaddr(const string&, const int);
const string to_string(const sockaddr_in*);

#endif
