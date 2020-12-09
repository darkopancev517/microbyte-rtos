#include "MicroByteHeap.h"
#include "Utils.h"

namespace microbyte {

Heap::Heap()
{
    Block &super = blockAt(superBlockOffset);
    super.setSize(superBlockSize);

    Block &first = blockRight(super);
    first.setSize(firstBlockSize);

    Block &guard = blockRight(first);
    guard.setSize(Block::guardBlockSize);

    super.setNext(blockOffset(first));
    first.setNext(blockOffset(guard));

    mMemory.mFreeSize = firstBlockSize;
}

void *Heap::calloc(size_t count, size_t allocSize)
{
    void *ret = nullptr;
    Block *prev = nullptr;
    Block *curr = nullptr;
    uint16_t size = static_cast<uint16_t>(count * allocSize);

    VERIFY_OR_EXIT(size);

    size += alignSize - 1 - blockRemainderSize;
    size &= ~(alignSize - 1);
    size += blockRemainderSize;

    prev = &blockSuper();
    curr = &blockNext(*prev);

    while (curr->getSize() < size)
    {
        prev = curr;
        curr = &blockNext(*curr);
    }

    VERIFY_OR_EXIT(curr->isFree());

    prev->setNext(curr->getNext());

    if (curr->getSize() > size + sizeof(Block))
    {
        const uint16_t newBlockSize = curr->getSize() - size - sizeof(Block);
        curr->setSize(size);

        Block &newBlock = blockRight(*curr);
        newBlock.setSize(newBlockSize);
        newBlock.setNext(0);

        if (prev->getSize() < newBlockSize)
        {
            blockInsert(*prev, newBlock);
        }
        else
        {
            blockInsert(blockSuper(), newBlock);
        }
        mMemory.mFreeSize -= sizeof(Block);
    }

    mMemory.mFreeSize -= curr->getSize();

    curr->setNext(0);

    memset(curr->getPointer(), 0, size);
    ret = curr->getPointer();

exit:
    return ret;
}

void Heap::blockInsert(Block &aPrev, Block &aBlock)
{
    Block *prev = &aPrev;
    for (Block *block = &blockNext(*prev);
         block->getSize() < aBlock.getSize();
         block = &blockNext(*block))
    {
        prev = block;
    }
    aBlock.setNext(prev->getNext());
    prev->setNext(blockOffset(aBlock));
}

Block &Heap::blockPrev(const Block &aBlock)
{
    Block *prev = &blockSuper();
    while (prev->getNext() != blockOffset(aBlock))
    {
        prev = &blockNext(*prev);
    }
    return *prev;
}

void Heap::free(void *ptr)
{
    if (ptr == nullptr)
    {
        return;
    }

    Block &block = blockOf(ptr);
    Block &right = blockRight(block);

    mMemory.mFreeSize += block.getSize();

    if (isLeftFree(block))
    {
        Block *prev = &blockSuper();
        Block *left = &blockNext(*prev);

        mMemory.mFreeSize += sizeof(Block);

        for (const uint16_t offset = block.getLeftNext();
             left->getNext() != offset;
             left = &blockNext(*left))
        {
            prev = left;
        }

        prev->setNext(left->getNext());
        left->setNext(0);

        if (right.isFree())
        {
            mMemory.mFreeSize += sizeof(Block);

            if (right.getSize() > left->getSize())
            {
                for (const uint16_t offset = blockOffset(right);
                     prev->getNext() != offset;
                     prev = &blockNext(*prev))
                {
                }
            }
            else
            {
                prev = &blockPrev(right);
            }

            prev->setNext(right.getNext());
            right.setNext(0);

            left->setSize(left->getSize() + right.getSize() + sizeof(Block));
        }

        left->setSize(left->getSize() + block.getSize() + sizeof(Block));

        blockInsert(*prev, *left);
    }
    else
    {
        if (right.isFree())
        {
            Block &prev = blockPrev(right);
            prev.setNext(right.getNext());
            block.setSize(block.getSize() + right.getSize() + sizeof(Block));
            blockInsert(prev, block);
            mMemory.mFreeSize += sizeof(Block);
        }
        else
        {
            blockInsert(blockSuper(), block);
        }
    }
}

} // namespace microbyte
