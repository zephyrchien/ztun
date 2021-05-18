#ifndef BUFFER_H_
#define BUFFER_H_

#include <array>
#include <memory>
#include "utils.h"


class Buffer
{
    public:
        bool ov;
        bool done;
        int r_offset;
        int w_offset;
        const static int size = 512;
        char* data;

        explicit Buffer();
        explicit Buffer(Buffer&&);
        Buffer& operator=(Buffer&&);
        ~Buffer();
        void* r_cursor();
        void* w_cursor();
};

typedef std::shared_ptr<Buffer> SharedBuffer;
typedef std::unique_ptr<Buffer> OwnedBuffer;

#endif