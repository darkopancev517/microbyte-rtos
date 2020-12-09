#ifndef PERIPH_TIMER_H
#define PERIPH_TIMER_H

#include "MicroBytePeriphTimer.h"

namespace microbyte {

typedef void (*TestPeriphTimerSet)(TimerClock *clock, uint32_t value);
typedef uint32_t (*TestPeriphTimerNow)(TimerClock *clock);
typedef void (*TestPeriphTimerCancel)(TimerClock *clock);
typedef void (*TestPeriphTimerIsrCallback)(void *arg, int channel);

void periphTimerInit(unsigned int dev, unsigned long freq, PeriphTimerIsrCallback callback, void *arg);

void periphTimerIsrCallback(void *arg, int channel);

void periphTimerSet(TimerClock *clock, uint32_t value);

uint32_t periphTimerNow(TimerClock *clock);

void periphTimerCancel(TimerClock *clock);

extern TestPeriphTimerSet gtestPeriphTimerSet;
extern TestPeriphTimerNow gtestPeriphTimerNow;
extern TestPeriphTimerCancel gtestPeriphTimerCancel;
extern TestPeriphTimerIsrCallback gtestPeriphTimerIsrCallback;

} // namespace microbyte

#endif /* PERIPH_TIMER_H */
