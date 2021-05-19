#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>

using std::string;

class Config
{
    public:
        const int local_port;
        const int remote_port;
        const string local_addr;
        const string remote_addr;
        Config(const string, const int, const string, const int);
        ~Config();
};

#endif
