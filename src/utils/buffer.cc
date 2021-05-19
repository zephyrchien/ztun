#include "buffer.h"


Buffer::Buffer(): done(false) { }

Buffer::~Buffer() { }

bool Buffer::is_done()
{
    return done;
}