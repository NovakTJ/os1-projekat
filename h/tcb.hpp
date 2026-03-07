//
// Created by marko on 20.4.22..
//

#ifndef OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP
#define OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP

#include "../lib/hw.h"
#include "../h/MemoryAllocator.h"
#include "scheduler.hpp"

// Thread Control Block
class TCB
{
public:
    void* operator new(size_t size) { return MemoryAllocator::allocateBytes(size); }
    void operator delete(void* ptr) { MemoryAllocator::deallocate((char*)ptr); }
    ~TCB() { if (stack) MemoryAllocator::deallocate((char*)stack); }

    bool isFinished() const { return finished; }

    void setFinished(bool value) { finished = value; }

    uint64 getTimeSlice() const { return timeSlice; }

    using BodyNoArg = void (*)();
    using BodyWithArg = void(*)(void*);

    // Kernel-internal: allocates stack itself
    static int createThread(TCB ** handle, BodyWithArg body, void* arg);

    // ABI-level: stack_space points to the last byte of pre-allocated stack
    static int createThread(TCB ** handle, BodyWithArg body, void* arg, void* stack_space);

    // Creates a bare TCB for the already-running thread (no stack alloc, no wrapper, no scheduler)
    static TCB* createForCurrent();

    static void urosDispatch();

    static void exit();

    static int createNonPreemptive(TCB ** handle, BodyWithArg body, void* arg);

    static TCB *running;

private:
    // Kernel-internal constructor: allocates stack
    TCB(BodyWithArg body, void* arg, uint64 timeSlice) :
            body(body),
            arg(arg),
            stack(body != nullptr ? (uint64*)MemoryAllocator::allocateBytes(STACK_SIZE * sizeof(uint64)) : nullptr),
            context({(uint64) &threadWrapper,
                     stack != nullptr ? (uint64) &stack[STACK_SIZE] : 0
                    }),
            timeSlice(timeSlice),
            finished(false)
    {
        if (body != nullptr) { Scheduler::put(this); }
    }

    // ABI constructor: uses pre-allocated stack (does NOT own it)
    TCB(BodyWithArg body, void* arg, void* stack_space, uint64 timeSlice) :
            body(body),
            arg(arg),
            stack(nullptr), // not owned, don't free in destructor
            context({(uint64) &threadWrapper,
                     stack_space != nullptr ? (uint64) stack_space : 0
                    }),
            timeSlice(timeSlice),
            finished(false)
    {
        if (body != nullptr) { Scheduler::put(this); }
    }

    struct Context //the rest of the context is kept on the stack
    {
        uint64 ra;
        uint64 sp;
    };

    BodyWithArg body;
    void* arg;
    uint64 *stack;
    Context context;
    uint64 timeSlice;
    bool finished;

    friend class Riscv;

    static void threadWrapper();

    static void nonPreemptiveWrapper();

    static void contextSwitch(Context *oldContext, Context *runningContext);

    static uint64 timeSliceCounter;

    static uint64 constexpr STACK_SIZE = 1024;
    static uint64 constexpr TIME_SLICE = 2;
};

#endif //OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP
