//
// Created by marko on 20.4.22..
//

#include "../h/scheduler.hpp"
#include "../h/tcb.hpp"

List<TCB> Scheduler::readyThreadQueue;

TCB *Scheduler::get()
{
    return readyThreadQueue.removeFirst();
}

void Scheduler::put(TCB *ccb)
{
    ccb->setCurrentQueue(1);
    readyThreadQueue.addLast(ccb);
}

List<TCB> SleepingQueue::sleepingThreadQueue;

TCB *SleepingQueue::get()
{
    return sleepingThreadQueue.removeFirst();
}

void SleepingQueue::put(TCB *ccb, int ticks)
{
    int remaining = ticks;
    int pos = 0;
    TCB *curr = sleepingThreadQueue.peekAt(pos);

    // Walk the list, subtracting each node's relative delta
    while (curr)
    {
        if (remaining < curr->getTimeUntilUnsleep())
        {
            // Insert before this node; adjust this node's delta
            curr->setTimeUntilUnsleep(curr->getTimeUntilUnsleep() - remaining);
            break;
        }
        remaining -= curr->getTimeUntilUnsleep();
        pos++;
        curr = sleepingThreadQueue.peekAt(pos);
    }

    ccb->setTimeUntilUnsleep(remaining);
    ccb->setCurrentQueue(2);
    sleepingThreadQueue.insertAt(pos, ccb);
}

void SleepingQueue::decrement()
{
    TCB *first = sleepingThreadQueue.peekFirst();
    if (!first) return;

    first->setTimeUntilUnsleep(first->getTimeUntilUnsleep() - 1);

    // Wake all threads whose time has come (delta == 0)
    while (sleepingThreadQueue.peekFirst() &&
           sleepingThreadQueue.peekFirst()->getTimeUntilUnsleep() <= 0)
    {
        Scheduler::put(sleepingThreadQueue.removeFirst());
    }
}