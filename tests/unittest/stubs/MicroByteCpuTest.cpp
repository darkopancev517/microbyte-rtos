#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#include "MicroByteCpu.h"
#include "MicroByteUnitTest.h"

using namespace microbyte;

void MicroByteCpuTest::disableIrq(void)
{
    assert(irqState == 0);

    irqState = 0xff;
}

void MicroByteCpuTest::enableIrq(void)
{
    irqState = 0;
}

void MicroByteCpuTest::restoreIrq(void)
{
    irqState = 0;
}

int MicroByteCpuTest::inIsr(void)
{
    return inIsrState;
}

void MicroByteCpuTest::endOfIsr(void)
{
}

void MicroByteCpuTest::triggerContextSwitch(void)
{
    contextSwitchState = 1;
}

void MicroByteCpuTest::contextExit(void)
{
}

void MicroByteCpuTest::sleep(int deep)
{
    (void)deep;
}

void MicroByteCpuTest::sleepUntilEvent(void)
{
}

void *MicroByteCpuTest::getMsp(void)
{
    return NULL;
}

char *MicroByteCpuTest::stackInit(ThreadFunc func, void *arg, void *stack, int size)
{
    (void)func;
    (void)arg;
    (void)stack;
    (void)size;

    return NULL;
}
