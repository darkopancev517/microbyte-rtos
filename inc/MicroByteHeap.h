#ifndef MICROBYTE_HEAP_H
#define MICROBYTE_HEAP_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "MicroByteRTOSConfig.h"

namespace microbyte {

class Block
{
    friend class Heap;

    enum
    {
        guardBlockSize = 0xffff,
    };

    uint16_t mSize;
    uint8_t mMemory[sizeof(uint16_t)];

    public:

    uint16_t getSize() const { return mSize; }

    void setSize(uint16_t size) { mSize = size; }

    uint16_t getNext() const
    {
        return *reinterpret_cast<const uint16_t *>(
            reinterpret_cast<const void *>(reinterpret_cast<const uint8_t *>(this) + sizeof(mSize) + mSize));
    }

    void setNext(uint16_t next)
    {
        *reinterpret_cast<uint16_t *>(
            reinterpret_cast<void *>(reinterpret_cast<uint8_t *>(this) + sizeof(mSize) + mSize)) = next;
    }

    void *getPointer() { return &mMemory; }

    uint16_t getLeftNext() const { return *(&mSize - 1); }

    bool isLeftFree(void) const { return getLeftNext() != 0; }

    bool isFree() const { return mSize != guardBlockSize && getNext() != 0; }
};

class Heap
{
    enum
    {
        memorySize = MICROBYTE_CONFIG_HEAP_SIZE,
        alignSize = sizeof(void *),
        blockRemainderSize = alignSize - sizeof(uint16_t) * 2,
        superBlockSize = alignSize - sizeof(Block),
        firstBlockSize = memorySize - alignSize * 3 + blockRemainderSize,
        superBlockOffset = alignSize - sizeof(uint16_t),
        firstBlockOffset = alignSize * 2 - sizeof(uint16_t),
        guardBlockOffset = memorySize - sizeof(uint16_t),
    };

    static_assert(memorySize % alignSize == 0, "Heap mMemory mSize is not aligned to Heap::alignSize!");

    union
    {
        uint16_t mFreeSize;
        long mLong[memorySize / sizeof(long)];
        uint8_t m8[memorySize];
        uint16_t m16[memorySize / sizeof(uint16_t)];
    } mMemory;

    Block &blockAt(uint16_t offset)
    {
        return *reinterpret_cast<Block *>(&mMemory.m16[offset / 2]);
    }

    Block &blockOf(void *ptr)
    {
        uint16_t offset = static_cast<uint16_t>(reinterpret_cast<uint8_t *>(ptr) - mMemory.m8);
        offset -= sizeof(uint16_t);
        return blockAt(offset);
    }

    Block &blockSuper() { return blockAt(superBlockOffset); }

    Block &blockNext(const Block &block) { return blockAt(block.getNext()); }

    Block &blockRight(const Block &block)
    {
        return blockAt(blockOffset(block) + sizeof(Block) + block.getSize());
    }

    Block &blockPrev(const Block &block);

    bool isLeftFree(const Block &block)
    {
        return (blockOffset(block) != firstBlockOffset && block.isLeftFree());
    }

    uint16_t blockOffset(const Block &block)
    {
        return static_cast<uint16_t>(reinterpret_cast<const uint8_t *>(&block) - mMemory.m8);
    }

    void blockInsert(Block &prev, Block &block);

    public:


    Heap();

    void *calloc(size_t count, size_t allocSize);

    void free(void *ptr);

    bool isClean() const
    {
        Heap &self = *const_cast<Heap *>(this);
        const Block &super = self.blockSuper();
        const Block &first = self.blockRight(super);
        return super.getNext() == self.blockOffset(first) && first.getSize() == firstBlockSize;
    }

    size_t getCapacity() const { return firstBlockSize; }

    size_t getFreeSize() const { return mMemory.mFreeSize; }
};

} // namespace microbyte


#endif /* MICROBYTE_HEAP_H */
