#ifndef CIB_H
#define CIB_H

#include <stdint.h>

namespace microbyte {

class Cib
{
    unsigned int readCount;
    unsigned int writeCount;
    unsigned int mask;

    public:

    Cib();

    void reset(unsigned int size);

    unsigned int avail();

    unsigned int getReadCount();

    unsigned int getWriteCount();

    unsigned int getMask();

    int getUnsafe();

    int putUnsafe();

    int full();

    int get();

    int peek();

    int put();
};

} // namespace microbyte

#endif /* CIB_H */
