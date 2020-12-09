#include "MicroByteTimer.h"
#include "MicroByteMutex.h"
#include "New.h"
#include "Utils.h"

namespace microbyte {

TimerClock::TimerClock(TimerOperations *func)
    : list()
    , ops(func)
    , last(NULL)
    , adjust(0)
{
    this->cpu = cpuGet();
}

Timer::Timer(TimerClock *clock, TimerCallback func, void *ptr)
    : base()
    , callback(func)
    , arg(ptr)
{
    this->clock = clock;
    this->cpu = cpuGet();
}

int TimerClock::isSet(Timer *t)
{
    if (!list.next)
    {
        return 0;
    }
    else
    {
        return (t->base.next || &t->base == last);
    }
}

void Timer::remove()
{
    cpu->disableIrq();
    if (clock->isSet(this))
    {
        clock->updateHeadOffset();
        clock->delEntryFromList(&this->base);
        clock->update();
    }
    cpu->restoreIrq();
}

void Timer::set(uint32_t value)
{
    cpu->disableIrq();
    clock->updateHeadOffset();
    if (clock->isSet(this))
    {
        clock->delEntryFromList(&this->base);
    }
    if (value > clock->adjust)
    {
        value -= clock->adjust;
    }
    else
    {
        value = 0;
    }
    base.offset = value;
    clock->addEntryToList(&base);
    if (clock->list.next == &base)
    {
        clock->ops->set(clock, value);
    }
    cpu->restoreIrq();
}

void TimerClock::addEntryToList(TimerBase *entry)
{
    uint32_t deltaSum = 0;
    TimerBase *clockList = &list;
    while (clockList->next)
    {
        TimerBase *listEntry = clockList->next;
        if ((listEntry->offset + deltaSum) > entry->offset)
        {
            break;
        }
        deltaSum += listEntry->offset;
        clockList = clockList->next;
    }
    entry->next = clockList->next;
    entry->offset -= deltaSum;
    if (entry->next)
    {
        entry->next->offset -= entry->offset;
    }
    else
    {
        last = entry;
    }
    clockList->next = entry;
}

void TimerClock::updateHeadOffset()
{
    uint32_t oldBase = list.offset;
    uint32_t clockNow = now();
    uint32_t diff = clockNow - oldBase;
    TimerBase *entry = list.next;
    if (entry)
    {
        do
        {
            if (diff <= entry->offset)
            {
                entry->offset -= diff;
                break;
            }
            else
            {
                diff -= entry->offset;
                entry->offset = 0;
                if (diff)
                {
                    do
                    {
                        entry = entry->next;
                    } while (entry && (entry->offset == 0));
                }
            }
        } while (diff && entry);
    }
    list.offset = clockNow;
}

void TimerClock::delEntryFromList(TimerBase *entry)
{
    TimerBase *clockList = &list;
    if (!isSet(reinterpret_cast<Timer *>(entry)))
    {
        return;
    }
    while (clockList->next)
    {
        TimerBase *listEntry = clockList->next;
        if (listEntry == entry)
        {
            if (entry == last)
            {
                last = (clockList == &list) ? NULL : clockList;
            }
            clockList->next = entry->next;
            if (clockList->next)
            {
                listEntry = clockList->next;
                listEntry->offset += entry->offset;
            }
            entry->next = NULL;
            break;
        }
        clockList = clockList->next;
    }
}

Timer *TimerClock::nowNext()
{
    TimerBase *entry = list.next;
    if (entry && (entry->offset == 0))
    {
        list.next = entry->next;
        if (!entry->next)
        {
            last = NULL;
        }
        return reinterpret_cast<Timer *>(entry);
    }
    else
    {
        return NULL;
    }
}

void TimerClock::update()
{
    if (list.next)
    {
        ops->set(this, list.next->offset);
    }
    else
    {
        ops->cancel(this);
    }
}

void TimerClock::handler()
{
    if (list.next == NULL)
    {
        return;
    }
    list.offset += list.next->offset;
    list.next->offset = 0;
    Timer *entry = nowNext();
    while (entry)
    {
        entry->callback(entry->arg);
        entry = nowNext();
        if (!entry)
        {
            updateHeadOffset();
            entry = nowNext();
        }
    }
    update();
    if (!cpu->inIsr())
    {
        cpu->triggerContextSwitch();
    }
}

static void unlockMutexCallback(void *arg)
{
    Mutex *mutex = static_cast<Mutex *>(arg);
    mutex->unlock();
}

void TimerClock::sleep(uint32_t duration)
{
    if (cpu->inIsr())
    {
        return;
    }
    Mutex mutex = Mutex();
    Timer timer = Timer(this, unlockMutexCallback, static_cast<void *>(&mutex));
    mutex.lock();
    timer.set(duration);
    mutex.lock();
}

void TimerClock::periodicWakeup(uint32_t *lastWakeup, uint32_t period)
{
    cpu->disableIrq();
    uint32_t clockNow = now();
    uint32_t target = *lastWakeup + period;
    uint32_t offset = target - clockNow;
    cpu->restoreIrq();
    if (offset <= period)
    {
        sleep(offset);
        *lastWakeup = target;
    }
    else
    {
        *lastWakeup = clockNow;
    }
}

} // namespace microbyte
