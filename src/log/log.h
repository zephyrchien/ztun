#ifndef LOG_H_
#define LOG_H_

#include <memory>
#include <chrono>
#include <ctime>
#include <cerrno>
#include <unistd.h>
#include "utils/utils.h"
#include "buffer/ringbuffer.h"

namespace chrono = std::chrono;

#define DEBUG(...) \
    if (Log::instance()->level >= Log::LEVEL::DEBUG) \
    Log::instance()->log(\
    Log::LEVEL::DEBUG, Log::BUF_LINE::LDEBUG, \
    "dbug", __VA_ARGS__)
#define INFO(...) \
    if (Log::instance()->level >= Log::LEVEL::INFO) \
    Log::instance()->log(\
    Log::LEVEL::INFO, Log::BUF_LINE::LINFO, \
    "info", __VA_ARGS__)
#define WARN(...) \
    if (Log::instance()->level >= Log::LEVEL::WARN) \
    Log::instance()->log(\
    Log::LEVEL::WARN, Log::BUF_LINE::LWARN, \
    "warn", __VA_ARGS__)

class Log
{
    public:
        enum LEVEL { NONE, WARN, INFO, DEBUG };
        enum BUF_LINE { LNONE, LWARN = 2, LINFO = 3, LDEBUG = 5};
        LEVEL level;
    
    private:
        const int fd_;
        const OwnedRingBuffer buf_;
        int line_;

    private:
        explicit Log(const int);
        static OwnedLogger logger_;        

    public:
        ~Log();
        static int init(LEVEL level, const int fd = STDERR_FILENO);
        static int init(LEVEL level, const char *file = "ztun.log");
        static OwnedLogger& instance();
        void set_level(LEVEL);

        template<typename ...Args>
        void log(LEVEL, BUF_LINE, const char*, const char*, Args...);
        template<typename ...Args>
        void debug(const char*, Args...);
        template<typename ...Args>
        void info(const char*, Args...);
        template<typename ...Args>
        void warn(const char*,Args...);
};

#endif