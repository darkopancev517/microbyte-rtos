#include "gtest/gtest.h"

#include "MicroByteHeap.h"

using namespace microbyte;

class TestMicroByteHeap : public testing::Test
{
    protected:

    Heap *heap;

    virtual void SetUp()
    {
        heap = new Heap();
    }

    virtual void TearDown()
    {
        delete heap;
    }
};

TEST_F(TestMicroByteHeap, constructorTest)
{
    EXPECT_TRUE(heap);
}

TEST_F(TestMicroByteHeap, allocateSingleTest)
{
    const size_t totalSize = heap->getFreeSize();

    {
        void *p = heap->calloc(1, 0);

        EXPECT_EQ(p, nullptr);
        EXPECT_EQ(totalSize, heap->getFreeSize());

        heap->free(p);

        p = heap->calloc(0, 1);

        EXPECT_EQ(p, nullptr);
        EXPECT_EQ(totalSize, heap->getFreeSize());

        heap->free(p);
    }

    for (size_t size = 1; size <= heap->getCapacity(); ++size)
    {
        void *p = heap->calloc(1, size);

        EXPECT_NE(p, nullptr);
        EXPECT_FALSE(heap->isClean());
        EXPECT_TRUE((heap->getFreeSize() + size) <= totalSize);

        memset(p, 0xff, size);

        heap->free(p);

        EXPECT_TRUE(heap->isClean());
        EXPECT_EQ(heap->getFreeSize(), totalSize);
    }
}

void testAllocateRandomly(Heap *heap, size_t sizeLimit, unsigned int seed)
{
    struct Node
    {
        Node *next;
        size_t size;
    };

    Node head;
    size_t nnodes = 0;

    srand(seed);

    const size_t totalSize = heap->getFreeSize();
    Node *last = &head;

    do
    {
        size_t size = sizeof(Node) + static_cast<size_t>(rand()) % sizeLimit;
        last->next = static_cast<Node *>(heap->calloc(1, size));

        if (last->next == nullptr)
        {
            break;
        }

        EXPECT_EQ(last->next->next, nullptr);

        last = last->next;
        last->size = size;
        ++nnodes;

        size_t freeIndex = static_cast<size_t>(rand()) % (nnodes * 2);

        if (freeIndex > nnodes)
        {
            freeIndex /= 2;
            Node *prev = &head;
            while (freeIndex--)
            {
                prev = prev->next;
            }
            Node *curr = prev->next;
            prev->next = curr->next;
            heap->free(curr);
            if (last == curr)
            {
                last = prev;
            }
            --nnodes;
        }
    } while (true);

    last = head.next;

    while (last)
    {
        Node *next = last->next;
        heap->free(last);
        last = next;
    }

    EXPECT_TRUE(heap->isClean());
    EXPECT_EQ(heap->getFreeSize(), totalSize);
}

TEST_F(TestMicroByteHeap, allocateMultipleTest)
{
    for (unsigned int seed = 0; seed < 10; ++seed)
    {
        size_t sizeLimit = (1 << seed);
        testAllocateRandomly(heap, sizeLimit, seed);
    }
}
