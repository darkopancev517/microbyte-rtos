#ifndef MICROBYTE_TIMER_H
#define MICROBYTE_TIMER_H

#include <stddef.h>
#include <stdint.h>

#include "MicroByteRTOSConfig.h"
#include "MicroByteCpu.h"

namespace microbyte {

typedef void (*TimerCallback)(void *arg);

class Timer;
class TimerClock;

class TimerBase
{
    friend class Timer;
    friend class TimerClock;

    protected:

    TimerBase *next;
    uint32_t offset;

    public:

    TimerBase()
        : next(NULL)
        , offset(0)
    {
    }
};

class TimerOperations
{
    public:

    void (*set)(TimerClock *clock, uint32_t value);
    uint32_t (*now)(TimerClock *clock);
    void (*cancel)(TimerClock *clock);

    TimerOperations()
    {
    }
};

class TimerClock
{
    friend class Timer;

    protected:

    TimerBase list;
    TimerOperations *ops;
    TimerBase *last;
    uint32_t adjust;

    MicroByteCpu *cpu;

    void updateHeadOffset();
    void addEntryToList(TimerBase *entry);
    void delEntryFromList(TimerBase *entry);
    void update();
    int isSet(Timer *t);
    Timer *nowNext();

    public:

    explicit TimerClock(TimerOperations *func);

    void handler();

    uint32_t now() { return ops->now(this); }

    void periodicWakeup(uint32_t *lastWakeup, uint32_t period);

    void sleep(uint32_t duration);
};

class Timer
{
    friend class TimerClock;

    protected:

    TimerBase base;
    TimerCallback callback;
    void *arg;

    TimerClock *clock;
    MicroByteCpu *cpu;

    public:

    explicit Timer(TimerClock *clock, TimerCallback func, void *ptr);

    void set(uint32_t value);

    void remove();
};

} // namespace microbyte

#endif /* MICROBYTE_TIMER_H */
