#include "periphTimer.h"

namespace microbyte {

TestPeriphTimerSet gtestPeriphTimerSet = nullptr;
TestPeriphTimerNow gtestPeriphTimerNow = nullptr;
TestPeriphTimerCancel gtestPeriphTimerCancel = nullptr;
TestPeriphTimerIsrCallback gtestPeriphTimerIsrCallback = nullptr;

void periphTimerInit(unsigned int dev, unsigned long freq, PeriphTimerIsrCallback callback, void *arg)
{
    (void)dev;
    (void)freq;
    (void)callback;
    (void)arg;
}

void periphTimerIsrCallback(void *arg, int channel)
{
    if (gtestPeriphTimerIsrCallback)
    {
        gtestPeriphTimerIsrCallback(arg, channel);
    }
    else
    {
        (void)arg;
        (void)channel;
    }
}

void periphTimerSet(TimerClock *clock, uint32_t value)
{
    if (gtestPeriphTimerSet)
    {
        gtestPeriphTimerSet(clock, value);
    }
    else
    {
        (void)clock;
        (void)value;
    }
}

uint32_t periphTimerNow(TimerClock *clock)
{
    if (gtestPeriphTimerNow)
    {
        return gtestPeriphTimerNow(clock);
    }
    else
    {
        (void)clock;
        return 0;
    }
}

void periphTimerCancel(TimerClock *clock)
{
    if (gtestPeriphTimerCancel)
    {
        gtestPeriphTimerCancel(clock);
    }
    else
    {
        (void)clock;
    }
}

} // namespace microbyte
