#include "../h/semaphore.hpp"
#include "../h/tcb.hpp"
#include "../h/scheduler.hpp"
#include "../h/riscv.hpp"
#include "../h/print.hpp"

_sem* _sem::allSems[MAX_SEMS] = {nullptr};

_sem::_sem(unsigned init) : works(true), value((int)init), blockedQueue() {
    for (int i = 0; i < MAX_SEMS; i++) {
        if (allSems[i] == nullptr) {
            allSems[i] = this;
            break;
        }
    }
}

_sem::~_sem() {
    // Semaphore is being closed - unblock all waiting threads
    // They will get a return value of -1 (error) from wait()
    works = false;
    while (!blockedQueue.empty()) {
        Scheduler::put(blockedQueue.get());
    }
    for (int i = 0; i < MAX_SEMS; i++) {
        if (allSems[i] == this) {
            allSems[i] = nullptr;
            break;
        }
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
    blockedQueue.put(old);
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

void _sem::print() {
    printKString("Sem @");
    printKHexInteger((uint64)this);
    printKString(" val=");
    printKInteger(value);
    printKString("\n");
    blockedQueue.print();
}

void _sem::printAll() {
    printKString("=== All Semaphores ===\n");
    int count = 0;
    for (int i = 0; i < MAX_SEMS; i++) {
        if (allSems[i] != nullptr) {
            printKString("[");
            printKInteger(i);
            printKString("] ");
            allSems[i]->print();
            count++;
        }
    }
    printKString("Total: ");
    printKInteger(count);
    printKString("\n");
}

int _sem::signal() {
    value += 1;
    if (!blockedQueue.empty()) {
        Scheduler::put(blockedQueue.get());
    }
    return 0;
}
