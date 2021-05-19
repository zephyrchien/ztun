#ifndef BUFFER_H_
#define BUFFER_H_

#include <memory>
#include "utils.h"


class Buffer
{
    public:
        bool done;

        explicit Buffer();
        virtual ~Buffer();
        bool is_done();
        virtual int read(const int, int&) = 0;
        virtual int write(const int, int&) = 0;
};

typedef std::shared_ptr<Buffer> SharedBuffer;
typedef std::unique_ptr<Buffer> OwnedBuffer;

#endif