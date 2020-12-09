#include "MicroByteMutex.h"

namespace microbyte {

Mutex::Mutex()
    : queue()
{
    this->cpu = cpuGet();
    this->scheduler = &ThreadScheduler::get();
}

Mutex::Mutex(CircList *locked)
    : queue()
{
    this->queue.next = locked;
    this->cpu = cpuGet();
    this->scheduler = &ThreadScheduler::get();
}

int Mutex::setLock(int blocking)
{
    cpu->disableIrq();
    if (queue.next == NULL)
    {
        queue.next = MICROBYTE_MUTEX_LOCKED;
        cpu->restoreIrq();
        return 1;
    }
    else if (blocking)
    {
        Thread *curThread = scheduler->activeThread();
        scheduler->setThreadStatus(curThread, MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
        if (queue.next == MICROBYTE_MUTEX_LOCKED)
        {
            queue.next = &curThread->runQueueEntry;
            queue.next->next = NULL;
        }
        else
        {
            curThread->addTo(&queue);
        }
        cpu->restoreIrq();
        cpu->triggerContextSwitch();
        return 1;
    }
    else
    {
        cpu->restoreIrq();
        return 0;
    }
}

ThreadPid Mutex::peek()
{
    cpu->disableIrq();
    if (queue.next == NULL || queue.next == MICROBYTE_MUTEX_LOCKED)
    {
        cpu->restoreIrq();
        return MICROBYTE_THREAD_PID_UNDEF;
    }
    Thread *thread = Thread::get(queue.next);
    cpu->restoreIrq();
    return thread->pid;
}

void Mutex::unlock()
{
    cpu->disableIrq();
    if (queue.next == NULL)
    {
        cpu->restoreIrq();
        return;
    }
    if (queue.next == MICROBYTE_MUTEX_LOCKED)
    {
        queue.next = NULL;
        cpu->restoreIrq();
        return;
    }
    CircList *head = queue.next;
    queue.next = head->next;    
    Thread *thread = Thread::get(head);
    scheduler->setThreadStatus(thread, MICROBYTE_THREAD_STATUS_PENDING);
    if (!queue.next)
    {
        queue.next = MICROBYTE_MUTEX_LOCKED;
    }
    cpu->restoreIrq();
    scheduler->contextSwitch(thread->priority);
}

void Mutex::unlockAndSleep()
{
    cpu->disableIrq();
    if (queue.next)
    {
        if (queue.next == MICROBYTE_MUTEX_LOCKED)
        {
            queue.next = NULL;
        }
        else
        {
            CircList *head = queue.next;
            queue.next = head->next;
            Thread *thread = Thread::get(head);
            scheduler->setThreadStatus(thread, MICROBYTE_THREAD_STATUS_PENDING);
            if (!queue.next)
            {
                queue.next = MICROBYTE_MUTEX_LOCKED;
            }
        }
    }
    cpu->restoreIrq();
    scheduler->sleep();
}

} // namespace microbyte
