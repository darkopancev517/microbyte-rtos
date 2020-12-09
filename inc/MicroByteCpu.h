#ifndef MICROBYTE_CPU_H
#define MICROBYTE_CPU_H

#include <stdint.h>

#include "MicroByteRTOSConfig.h"
#include "MicroByteThread.h"

namespace microbyte {

class MicroByteCpu
{
    protected:

    unsigned irqState;

    public:

    MicroByteCpu()
    {
        this->irqState = 0;
    }

    virtual void disableIrq()
    {
    }

    virtual void enableIrq()
    {
    }

    virtual void restoreIrq()
    {
    }

    virtual int inIsr()
    {
        return 0;
    }

    virtual void endOfIsr()
    {
    }

    virtual void triggerContextSwitch()
    {
    }

    virtual void contextExit()
    {
    }

    virtual void sleep(int deep)
    {
        (void)deep;
    }

    virtual void sleepUntilEvent()
    {
    }

    virtual void *getMsp()
    {
        return NULL;
    }

    virtual char *stackInit(ThreadFunc func, void *arg, void *stack, int size)
    {
        (void)func;
        (void)arg;
        (void)stack;
        (void)size;

        return NULL;
    }
};

void cpuSet(MicroByteCpu *cpu);

MicroByteCpu *cpuGet();

} // namespace microbyte

#endif /* MICROBYTE_CPU_H */
