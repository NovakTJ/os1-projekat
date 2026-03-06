//
// Created by marko on 20.4.22..
//

#include "../h/tcb.hpp"
#include "../h/riscv.hpp"


//TODO: find the semaphore from the videos
TCB *TCB::running = nullptr;

uint64 TCB::timeSliceCounter = 0;

int TCB::createThread(TCB ** handle, BodyWithArg body, void* arg)
{
    *handle = new TCB(body, arg, TIME_SLICE);
    return (*handle != nullptr);
}

void TCB::yield()
{
    //TODO: this is compatible with uros's code. not sure if its compatible w the final code. it is if the required thread_dispatch syscall calls this function.
    //in Uros's code this function is called only from superviser mode and not from usermain() im 80% sure. but it should work even if it's called from user mode becaue it elevates.
    Riscv::ecall(0x13);
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
    running->setFinished(true);
    TCB::yield();
}
