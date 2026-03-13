//
// Created by marko on 20.4.22..
//

#ifndef OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP
#define OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP

#include "../lib/hw.h"
#include "../h/MemoryAllocator.hpp"
#include "scheduler.hpp"

inline void pushCalleeSaved() {
    __asm__ volatile(
        "addi sp, sp, -96\n"
        "sd s0,  0*8(sp)\n"
        "sd s1,  1*8(sp)\n"
        "sd s2,  2*8(sp)\n"
        "sd s3,  3*8(sp)\n"
        "sd s4,  4*8(sp)\n"
        "sd s5,  5*8(sp)\n"
        "sd s6,  6*8(sp)\n"
        "sd s7,  7*8(sp)\n"
        "sd s8,  8*8(sp)\n"
        "sd s9,  9*8(sp)\n"
        "sd s10, 10*8(sp)\n"
        "sd s11, 11*8(sp)\n"
    );
}

inline void popCalleeSaved() {
    __asm__ volatile(
        "ld s0,  0*8(sp)\n"
        "ld s1,  1*8(sp)\n"
        "ld s2,  2*8(sp)\n"
        "ld s3,  3*8(sp)\n"
        "ld s4,  4*8(sp)\n"
        "ld s5,  5*8(sp)\n"
        "ld s6,  6*8(sp)\n"
        "ld s7,  7*8(sp)\n"
        "ld s8,  8*8(sp)\n"
        "ld s9,  9*8(sp)\n"
        "ld s10, 10*8(sp)\n"
        "ld s11, 11*8(sp)\n"
        "addi sp, sp, 96\n"
    );
}


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

    static void kDispatch();
    static int putCurrentToSleep(uint64 ticks);
    static void unsleepFirst();
    static void exit();

    static int createNonPreemptive(TCB ** handle, BodyWithArg body, void* arg);

    static int createKernelThread(TCB ** handle, BodyWithArg body, void* arg);

    static TCB *running;
    int getTimeUntilUnsleep() const { return timeUntilUnsleep; }
    void setTimeUntilUnsleep(int ticks) { timeUntilUnsleep = ticks; }
    static void OThreadBody(void* arg);

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
            finished(false),
            timeUntilUnsleep(0)
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
            finished(false),
            timeUntilUnsleep(0)
    {
        if (body != nullptr) { Scheduler::put(this); }
    }



    BodyWithArg body;
    void* arg;
    uint64 *stack;
public:
    struct Context //the rest of the context is kept on the stack
    {
        uint64 ra;
        uint64 sp;
    };
    Context context;
    static void contextSwitch(Context *oldContext, Context *runningContext);
private:
    uint64 timeSlice;
    bool finished;
    int timeUntilUnsleep;

    friend class Riscv;

    static void threadWrapper();

    static void nonPreemptiveWrapper();

    static void kernelThreadWrapper();

    static uint64 timeSliceCounter;

    static uint64 constexpr STACK_SIZE = 1024;
    static uint64 constexpr TIME_SLICE = 2;
};

#endif //OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP
