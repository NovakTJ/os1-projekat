//
// Created by marko on 20.4.22..
//

#include "../h/tcb.hpp"
#include "../h/riscv.hpp"
#include "../h/syscall_c.h"

//TODO: find the semaphore from the videos
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



void TCB::urosDispatch()
{
    //This is called by the kernel I think - not by usermode theread_dispathc, - the naming is unfortunate.
    TCB *old = running;
    if (!old->isFinished()) { Scheduler::put(old); }
    running = Scheduler::get();

    TCB::contextSwitch(&old->context, &running->context);
}

void TCB::threadWrapper()
{
    Riscv::popSppSpie(); //brings down the priv level with sret black magic
    running->body(running->arg); //does the fn
    thread_exit();
}
