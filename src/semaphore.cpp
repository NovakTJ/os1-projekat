#include "../h/semaphore.hpp"
#include "../h/tcb.hpp"

_sem::_sem(unsigned init) : value((int)init), blockedQueue() {
    // TODO: implement
}

_sem::~_sem() {
    while (!blockedQueue.empty())
    {
        //TODO: many context switches will happen here!
    }
}

int _sem::open(_sem** handle, unsigned init) {
    // TODO: implement
    return 0;
}

int _sem::close(_sem* handle) {
    // TODO: implement
    return 0;
}
int _sem::putIntoQueue()
{
    TCB *old = TCB::running;
    blockedQueue.put(old);
    TCB *next = Scheduler::get();
    if (next != nullptr) {
        TCB::running = next;
        TCB::contextSwitch(&old->context, &TCB::running->context);
    }
    // else: queue empty, keep running current thread
}
int _sem::wait() {
    //no need for sync, no interrupts here
    value-=1;
    if (value<0)
    {
        putIntoQueue();
    }
    if (works) return 0;
    else return -1;
}

int _sem::signal() {
    value+=1;
    if (!blockedQueue.empty())
    {
        Scheduler::put(blockedQueue.get());
    }
    return 0;
}
