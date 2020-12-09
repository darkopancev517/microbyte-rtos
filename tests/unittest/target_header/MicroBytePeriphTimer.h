#ifndef MICROBYTE_PERIPH_TIMER_H
#define MICROBYTE_PERIPH_TIMER_H

#include "MicroByteRTOSConfig.h"
#include "MicroByteTimer.h"

namespace microbyte {

typedef void (*PeriphTimerIsrCallback)(void *arg, int channel);

class PeriphTimer
{
    TimerClock super;
    unsigned int dev;
    uint16_t minValue;

    public:

    explicit PeriphTimer(TimerOperations *periphTimerOps,
                         unsigned int dev,
                         uint16_t min,
                         unsigned long freq);

    TimerClock *clock() { return &super; }

    uint16_t min() { return minValue; }
};

} // namespace microbyte

#endif /* MICROBYTE_PERIPH_TIMER_H */
