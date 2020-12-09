#include "MicroByteEvent.h"

namespace microbyte {

Event::Event()
{
    this->node.next = NULL;
}

EventQueue::EventQueue()
{
    this->queue.next = NULL;
    this->cpu = cpuGet();
    this->scheduler = &ThreadScheduler::get();
}

void EventQueue::post(Event *event, Thread *thread)
{
    if (event == NULL || thread == NULL)
    {
        return;
    }
    cpu->disableIrq();
    if (!event->node.next)
    {
        queue.rightPush(&event->node);
    }
    cpu->restoreIrq();
    scheduler->setThreadFlags(thread, MICROBYTE_EVENT_THREAD_FLAG);
}

void EventQueue::cancel(Event *event)
{
    if (event == NULL)
    {
        return;
    }
    cpu->disableIrq();
    queue.remove(&event->node);
    event->node.next = NULL;
    cpu->restoreIrq();
}

Event *EventQueue::get()
{
    cpu->disableIrq();
    Event *result = reinterpret_cast<Event *>(queue.leftPop());
    cpu->restoreIrq();
    if (result)
    {
        result->node.next = NULL;
    }
    return result;
}

Event *EventQueue::wait()
{
    Event *result = NULL;
#ifdef UNITTEST
    cpu->disableIrq();
    result = reinterpret_cast<Event *>(queue.leftPop());
    cpu->restoreIrq();
    if (result == NULL)
    {
        scheduler->waitAnyThreadFlags(MICROBYTE_EVENT_THREAD_FLAG);
    }
#else
    do
    {
        cpu->disableIrq();
        result = reinterpret_cast<Event *>(queue.leftPop());
        cpu->restoreIrq();
        if (result == NULL)
        {
            scheduler->waitAnyThreadFlags(MICROBYTE_EVENT_THREAD_FLAG);
        }
    } while (result == NULL);
#endif
    return result;
}

int EventQueue::release(Event *event)
{
    // Before releasing the event, make sure it's no longer in the event queue
    if (queue.find(reinterpret_cast<CircList *>(event)) == NULL)
    {
        event->node.next = NULL;
        return 1;
    }
    else
    {
        return -1;
    }
}

int EventQueue::pending()
{
    return queue.count();
}

Event *EventQueue::peek()
{
    return reinterpret_cast<Event *>(queue.leftPeek());
}

} // namespace microbyte
