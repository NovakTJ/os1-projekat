//
// Created by marko on 20.4.22..
//

#include "../h/tcb.hpp"
#include "../h/riscv.hpp"
#include "../h/syscall_c.h"

TCB *TCB::running = nullptr;

uint64 TCB::timeSliceCounter = 0;

int TCB::createThread(TCB ** handle, BodyWithArg body, void* arg)
{
    *handle = new TCB(body, arg, TIME_SLICE);
    if (*handle == nullptr) return -1;
    return 0;
}

int TCB::createThread(TCB ** handle, BodyWithArg body, void* arg, void* stack_space)
{
    *handle = new TCB(body, arg, stack_space, TIME_SLICE);
    if (*handle == nullptr) return -1;
    return 0;
}

TCB* TCB::createForCurrent()
{
    // nullptr body => no stack alloc, no wrapper, no scheduler insertion
    TCB* tcb = new TCB(nullptr, nullptr, TIME_SLICE);
    return tcb;
}



void TCB::kDispatch()
{
    auto volatile ksepc = Riscv::r_sepc();
    auto volatile ksstatus = Riscv::r_sstatus();
    TCB *old = running;
    if (!old->isFinished()) { Scheduler::put(old); }
    TCB *next = Scheduler::get();
    if (next != nullptr) {
        running = next;
        pushCalleeSaved();
        TCB::contextSwitch(&old->context, &running->context);
        popCalleeSaved();
    }
    // else: queue empty, keep running current thread
    Riscv::w_sstatus(ksstatus);
    Riscv::w_sepc(ksepc);
}

void unsleepFirst()
{
    Scheduler::put(SleepingQueue::get());
}

int TCB::putCurrentToSleep(uint64 ticks)
{
    auto volatile ksepc = Riscv::r_sepc();
    auto volatile ksstatus = Riscv::r_sstatus();
    TCB *old = running;
    if (!old->isFinished()) { SleepingQueue::put(old, ticks); }
    TCB *next = Scheduler::get();
    if (next != nullptr) {
        running = next;
        pushCalleeSaved();
        TCB::contextSwitch(&old->context, &running->context);
        popCalleeSaved();
    }
    // else: queue empty, keep running current thread
    Riscv::w_sstatus(ksstatus);
    Riscv::w_sepc(ksepc);
    return 0;
}

int TCB::createNonPreemptive(TCB ** handle, BodyWithArg body, void* arg)
{
    *handle = new TCB(body, arg, TIME_SLICE);
    if (*handle == nullptr) return -1;
    (*handle)->context.ra = (uint64)&nonPreemptiveWrapper;
    return 0;
}

int TCB::createKernelThread(TCB ** handle, BodyWithArg body, void* arg)
{
    *handle = new TCB(body, arg, TIME_SLICE);
    if (*handle == nullptr) return -1;
    (*handle)->context.ra = (uint64)&kernelThreadWrapper;
    return 0;
}

void TCB::threadWrapper()
{
    Riscv::popSppSpie(); //brings down the priv level with sret black magic
    running->body(running->arg); //does the fn
    thread_exit();
}

void TCB::nonPreemptiveWrapper()
{
    Riscv::mc_sstatus(Riscv::SSTATUS_SPIE);
    Riscv::popSppSpie();
    running->body(running->arg);
    thread_exit();
}

void TCB::kernelThreadWrapper()
{
    running->body(running->arg);
    TCB::running->setFinished(true);
    TCB::kDispatch();
}