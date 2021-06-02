#include "zbuffer.h"


ZBuffer::ZBuffer(): done(false), offset(0)
{
    int rw_pipe[2];
    int ret __attribute__((unused)) = pipe2(rw_pipe, O_NONBLOCK);
    rfd = rw_pipe[0];
    wfd = rw_pipe[1];
}

ZBuffer::~ZBuffer()
{
    close(rfd);
    close(wfd);
}
