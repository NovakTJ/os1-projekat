#include "../h/semaphore.hpp"
#include "../h/tcb.hpp"
#include "../h/scheduler.hpp"
#include "../h/riscv.hpp"

_sem::_sem(unsigned init) : works(true), value((int)init), blockedQueue() {
}

_sem::~_sem() {
    // Semaphore is being closed - unblock all waiting threads
    // They will get a return value of -1 (error) from wait()
    works = false;
    while (!blockedQueue.empty()) {
        TCB* t = blockedQueue.removeFirst();
        Scheduler::put(t);
    }
}

int _sem::open(_sem** handle, unsigned init) {
    *handle = new _sem(init);
    if (*handle == nullptr) return -1;
    return 0;
}

int _sem::close(_sem* handle) {
    if (handle == nullptr) return -1;
    delete handle;
    return 0;
}

int _sem::putIntoQueue() {
    auto volatile ksepc = Riscv::r_sepc();
    auto volatile ksstatus = Riscv::r_sstatus();
    TCB *old = TCB::running;
    old->setCurrentQueue(3);
    blockedQueue.addLast(old);
    TCB *next = Scheduler::get();
    if (next != nullptr) {
        TCB::running = next;
        pushCalleeSaved();
        TCB::contextSwitch(&old->context, &TCB::running->context);
        popCalleeSaved();
    }
    Riscv::w_sstatus(ksstatus);
    Riscv::w_sepc(ksepc);
    return 0;
}

int _sem::wait() {
    // No need for sync, interrupts are disabled here (inside syscall handler)
    value -= 1;
    if (value < 0) {
        putIntoQueue();
    }
    if (works) return 0;
    else return -1;
}

int _sem::signal() {
    value += 1;
    if (!blockedQueue.empty()) {
        Scheduler::put(blockedQueue.removeFirst());
    }
    return 0;
}
