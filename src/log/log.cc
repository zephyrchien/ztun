#include "log.h"


OwnedLogger Log::logger_ = OwnedLogger(nullptr);

Log::Log(const int fd): fd_(fd), buf_(new RingBuffer()) { }

Log::~Log()
{
    int x;
    buf_->xwrite(fd_, x);
    close(fd_);
}

int Log::init(LEVEL level, const int fd)
{
    int fd2 = dup_with_opt(fd);
    if (fd2 < 0) return -1;
    set_nonblocking(fd2);
    Log::logger_ = OwnedLogger(new Log(fd2));
    logger_->level = level;
    return 0;
}

int Log::init(LEVEL level, const char *file)
{
    int fd = open(file, O_WRONLY|O_CREAT|O_APPEND, 0664);
    if (fd < 0) return -1;
    Log::logger_ = OwnedLogger(new Log(fd));
    logger_->level = level;
    return 0;
}

OwnedLogger& Log::instance()
{
    return Log::logger_;
}


template<typename ...Args>
void Log::log(LEVEL level, BUF_LINE line, const char* lv,
    const char* fmt, Args ...args)
{
    // if (level_ < level) return;
    const static int fmt_size = 64;
    const static int time_size = 64;
    const static int text_size = 128;
    static char fmt_buf[fmt_size];
    static char time_buf[time_size];
    static char text_buf[text_size];

    auto tp = chrono::system_clock::now();
    time_t now = chrono::system_clock::to_time_t(tp);
    tm *time_info = localtime(&now);
    strftime(time_buf, time_size, "%m/%d %H:%M:%S", time_info);

    // fmt: [time][level]|text
    snprintf(fmt_buf, fmt_size, "[%%s][%%s]|%s", fmt);
    int n = snprintf(text_buf, text_size, fmt_buf, time_buf, lv, args...);
    int ret = buf_->xread(text_buf, n);
    if (ret == 0) line_++;
    if (line_ > line) 
    {
        buf_->xwrite(fd_, ret);
        (ret == 0) && (line_ = 0);
    }
}

template<typename ...Args>
void Log::debug(const char* fmt, Args ...args)
{
    return log(LEVEL::DEBUG, BUF_LINE::LDEBUG, "dbug", fmt, args...);
}

template<typename ...Args>
void Log::info(const char* fmt, Args ...args)
{
    return log(LEVEL::INFO, BUF_LINE::LINFO, "info", fmt, args...);
}

template<typename ...Args>
void Log::warn(const char* fmt, Args ...args)
{
    return log(LEVEL::WARN, BUF_LINE::LWARN, "warn", fmt, args...);
}

// force template install
// %d
template void Log::log(LEVEL, BUF_LINE, const char*, const char*,
    int);
// %s
template void Log::log(LEVEL, BUF_LINE, const char*, const char*,
    const char*);
// %d %d
template void Log::log(LEVEL, BUF_LINE, const char*, const char*,
    int, int);
// %d %s
template void Log::log(LEVEL, BUF_LINE, const char*, const char*,
    int, const char*);
// %s %s
template void Log::log(LEVEL, BUF_LINE, const char*, const char*,
    const char*, const char*);
// %d %d %s
template void Log::log(LEVEL, BUF_LINE, const char*, const char*,
    int, int, const char*);
// %d %s %s
template void Log::log(LEVEL, BUF_LINE, const char*, const char*,
    int, const char*, const char*);
// %s %s %s
template void Log::log(LEVEL, BUF_LINE, const char*, const char*,
    const char*, const char*, const char*);