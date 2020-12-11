#include "MicroByteMsg.h"

namespace microbyte {

Msg::Msg()
    : senderPid(MICROBYTE_THREAD_PID_UNDEF)
    , type(0)
{
    this->content.ptr = NULL;
    this->content.value = 0;
    this->cpu = uByteCpu;
    this->scheduler = &ThreadScheduler::get();
}

int Msg::send(ThreadPid targetPid, int blocking, unsigned state)
{
    Thread *targetThread = scheduler->threadFromContainer(targetPid);
    senderPid = scheduler->activePid();
    if (targetThread == NULL || !targetThread->hasMsgQueue())
    {
        cpu->restoreIrq(state);
        return -1;
    }
    Thread *curThread = scheduler->activeThread();
    if (targetThread->status != MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED)
    {
        if (targetThread->queuedMsg(this))
        {
            cpu->restoreIrq(state);
            if (curThread->getStatus() == MICROBYTE_THREAD_STATUS_REPLY_BLOCKED)
            {
                cpu->triggerContextSwitch();
            }
            return 1;
        }
        if (!blocking)
        {
            cpu->restoreIrq(state);
            return 0;
        }
        curThread->waitData = static_cast<void *>(this);
        ThreadStatus newStatus;
        if (curThread->status == MICROBYTE_THREAD_STATUS_REPLY_BLOCKED)
        {
            newStatus = MICROBYTE_THREAD_STATUS_REPLY_BLOCKED;
        }
        else
        {
            newStatus = MICROBYTE_THREAD_STATUS_SEND_BLOCKED;
        }
        scheduler->setThreadStatus(curThread, newStatus);
        curThread->addTo(reinterpret_cast<CircList *>(&targetThread->msgWaiters));
        cpu->restoreIrq(state);
        cpu->triggerContextSwitch();
    }
    else
    {
        Msg *targetMsg = static_cast<Msg *>(targetThread->waitData);
        *targetMsg = *this;
        scheduler->setThreadStatus(targetThread, MICROBYTE_THREAD_STATUS_PENDING);
        cpu->restoreIrq(state);
        cpu->triggerContextSwitch();
    }
    return 1;
}

int Msg::receive(int blocking)
{
    unsigned state = cpu->disableIrq();
    Thread *curThread = scheduler->activeThread();
    if (curThread == NULL || !curThread->hasMsgQueue())
    {
        cpu->restoreIrq(state);
        return -1;
    }
    int queueIndex = -1;
    if (curThread->msgArray != NULL)
    {
        queueIndex = curThread->msgQueue.get();
    }
    if (!blocking && (!curThread->msgWaiters.next && queueIndex == -1))
    {
        cpu->restoreIrq(state);
        return -1;
    }
    if (queueIndex >= 0)
    {
        *this = *static_cast<Msg *>(&curThread->msgArray[queueIndex]);
        cpu->restoreIrq(state);
        return 1;
    }
    else
    {
        curThread->waitData = static_cast<void *>(this);
    }
    List *next = curThread->msgWaiters.removeHead();
    if (next == NULL)
    {
        if (queueIndex < 0)
        {
            scheduler->setThreadStatus(curThread, MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED);
            cpu->restoreIrq(state);
            cpu->triggerContextSwitch();
        }
        else
        {
            cpu->restoreIrq(state);
        }
        return 1;
    }
    else
    {
        Thread *senderThread = Thread::get(reinterpret_cast<CircList *>(next));
        Msg *tmp = NULL;
        if (queueIndex >= 0)
        {
            tmp = &curThread->msgArray[curThread->msgQueue.put()];
        }
        Msg *senderMsg = static_cast<Msg *>(senderThread->waitData);
        if (tmp != NULL)
        {
            *tmp = *senderMsg;
            *this = *tmp;
        }
        else
        {
            *this = *senderMsg;
        }
        uint8_t senderPrio = MICROBYTE_THREAD_PRIORITY_IDLE;
        if (senderThread->status != MICROBYTE_THREAD_STATUS_REPLY_BLOCKED)
        {
            senderThread->waitData = NULL;
            scheduler->setThreadStatus(senderThread, MICROBYTE_THREAD_STATUS_PENDING);
            senderPrio = senderThread->priority;
        }
        cpu->restoreIrq(state);
        if (senderPrio < MICROBYTE_THREAD_PRIORITY_IDLE)
        {
            scheduler->contextSwitch(senderPrio);
        }
        return 1;
    }
}

int Msg::send(ThreadPid targetPid)
{
    if (cpu->inIsr())
    {
        return sendInIsr(targetPid);
    }
    if (scheduler->activePid() == targetPid)
    {
        return sendToSelf();
    }
    return send(targetPid, 1, cpu->disableIrq());
}

int Msg::trySend(ThreadPid targetPid)
{
    if (cpu->inIsr())
    {
        return sendInIsr(targetPid);
    }
    if (scheduler->activePid() == targetPid)
    {
        return sendToSelf();
    }
    return send(targetPid, 0, cpu->disableIrq());
}

int Msg::sendToSelf(void)
{
    unsigned state = cpu->disableIrq();
    Thread *curThread = scheduler->activeThread();
    if (curThread == NULL || !curThread->hasMsgQueue())
    {
        cpu->restoreIrq(state);
        return -1;
    }
    senderPid = curThread->pid;
    int result = curThread->queuedMsg(this);
    cpu->restoreIrq(state);
    return result;
}

int Msg::sendInIsr(ThreadPid targetPid)
{
    Thread *targetThread = scheduler->threadFromContainer(targetPid);
    if (targetThread == NULL || !targetThread->hasMsgQueue())
    {
        return -1;
    }
    senderPid = MICROBYTE_THREAD_PID_ISR;
    if (targetThread->status == MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED)
    {
        Msg *targetMsg = static_cast<Msg *>(targetThread->waitData);
        *targetMsg = *this;
        scheduler->setThreadStatus(targetThread, MICROBYTE_THREAD_STATUS_PENDING);
        scheduler->requestContextSwitch();
        return 1;
    }
    else
    {
        return targetThread->queuedMsg(this);
    }
}

int Msg::sentByIsr()
{
    return senderPid == MICROBYTE_THREAD_PID_ISR;
}

int Msg::receive()
{
    return receive(1);
}

int Msg::tryReceive()
{
    return receive(0);
}

int Msg::sendReceive(Msg *reply, ThreadPid targetPid)
{
    unsigned state = cpu->disableIrq();
    Thread *curThread = scheduler->activeThread();
    if (curThread->pid == targetPid || !curThread->hasMsgQueue())
    {
        return -1;
    }
    scheduler->setThreadStatus(curThread, MICROBYTE_THREAD_STATUS_REPLY_BLOCKED);
    curThread->waitData = static_cast<void *>(reply);
    *reply = *this;
    return reply->send(targetPid, 1, state);
}

int Msg::reply(Msg *reply)
{
    unsigned state = cpu->disableIrq();
    Thread *targetThread = scheduler->threadFromContainer(senderPid);
    if (targetThread == NULL ||
        !targetThread->hasMsgQueue() ||
        targetThread->status != MICROBYTE_THREAD_STATUS_REPLY_BLOCKED)
    {
        cpu->restoreIrq(state);
        return -1;
    }
    Msg *targetMsg = static_cast<Msg *>(targetThread->waitData);
    *targetMsg = *reply;
    scheduler->setThreadStatus(targetThread, MICROBYTE_THREAD_STATUS_PENDING);
    cpu->restoreIrq(state);
    scheduler->contextSwitch(targetThread->priority);
    return 1;
}

int Msg::replyInIsr(Msg *reply)
{
    Thread *targetThread = scheduler->threadFromContainer(senderPid);
    if (targetThread == NULL ||
        !targetThread->hasMsgQueue() ||
        targetThread->status != MICROBYTE_THREAD_STATUS_REPLY_BLOCKED)
    {
        return -1;
    }
    Msg *targetMsg = static_cast<Msg *>(targetThread->waitData);
    *targetMsg = *reply;
    scheduler->setThreadStatus(targetThread, MICROBYTE_THREAD_STATUS_PENDING);
    scheduler->requestContextSwitch();
    return 1;
}

} // namespace microbyte
