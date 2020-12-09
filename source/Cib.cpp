#include <stddef.h>
#include <stdint.h>

#include "Cib.h"

namespace microbyte {

Cib::Cib()
    : readCount(0)
    , writeCount(0)
    , mask(0)
{
}

void Cib::reset(unsigned int size)
{
    if ((size & (size - 1)))
    {
        while (1);
    }
    readCount = 0;
    writeCount = 0;
    mask = size - 1;
}

unsigned int Cib::avail()
{
    return writeCount - readCount;
}

unsigned int Cib::getReadCount()
{
    return readCount;
}

unsigned int Cib::getWriteCount()
{
    return writeCount;
}

unsigned int Cib::getMask()
{
    return mask;
}

int Cib::getUnsafe()
{
    return static_cast<int>(readCount++ & mask);
}

int Cib::putUnsafe()
{
    return static_cast<int>(writeCount++ & mask);
}

int Cib::full()
{
    return avail() > mask;
}

int Cib::get()
{
    if (avail())
    {
        return static_cast<int>(readCount++ & mask);
    }
    return -1;
}

int Cib::peek()
{
    if (avail())
    {
        return static_cast<int>(readCount & mask);
    }
    return -1;
}

int Cib::put()
{
    if (avail() <= static_cast<int>(mask))
    {
        return static_cast<int>(writeCount++ & mask);
    }
    return -1;
}

} // namespace microbyte
