#include "MicroBytePeriphTimer.h"
#include "periphTimer.h"

namespace microbyte {

PeriphTimer::PeriphTimer(TimerOperations *periphTimerOps,
                         unsigned int dev,
                         uint16_t min,
                         unsigned long freq)
    : super(periphTimerOps)
    , dev(dev)
    , minValue(min)
{
    periphTimerInit(dev, freq, periphTimerIsrCallback, static_cast<void *>(this));
}

} // namespace microbyte
