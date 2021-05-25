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

#define DEBUG(...) Log::instance()->log(\
    Log::LEVEL::DEBUG, Log::BUF_LINE::LDEBUG, \
    "debug", __VA_ARGS__)
#define INFO(...) Log::instance()->log(\
    Log::LEVEL::INFO, Log::BUF_LINE::LINFO, \
    "info", __VA_ARGS__)
#define WARN(...) Log::instance()->log(\
    Log::LEVEL::WARN, Log::BUF_LINE::LWARN, \
    "warn", __VA_ARGS__)

class Log
{
    public:
        enum LEVEL { NONE, WARN, INFO, DEBUG };
        enum BUF_LINE { LNONE, LWARN = 2, LINFO = 5, LDEBUG = 10};
    
    private:
        LEVEL level_;
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