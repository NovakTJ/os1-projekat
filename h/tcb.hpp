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
    void* operator new(size_t size) { return MemoryAllocator::allocate(size); }
    void operator delete(void* ptr) { MemoryAllocator::deallocate((char*)ptr); }
    ~TCB() { if (stack) MemoryAllocator::deallocate((char*)stack); }

    bool isFinished() const { return finished; }

    void setFinished(bool value) { finished = value; }

    uint64 getTimeSlice() const { return timeSlice; }

    using Body = void (*)();

    static TCB *createThread(Body body);

    static void yield();

    static TCB *running;

private:
    TCB(Body body, uint64 timeSlice) :
            body(body),
            stack(body != nullptr ? (uint64*)MemoryAllocator::allocate(STACK_SIZE * sizeof(uint64)) : nullptr),
            context({(uint64) &threadWrapper,
                     stack != nullptr ? (uint64) &stack[STACK_SIZE] : 0
                    }),
            timeSlice(timeSlice),
            finished(false)
    {
        if (body != nullptr) { Scheduler::put(this); }
    }

    struct Context
    {
        uint64 ra;
        uint64 sp;
    };

    Body body;
    uint64 *stack;
    Context context;
    uint64 timeSlice;
    bool finished;

    friend class Riscv;

    static void threadWrapper();

    static void contextSwitch(Context *oldContext, Context *runningContext);

    static void urosDispatch();

    static uint64 timeSliceCounter;

    static uint64 constexpr STACK_SIZE = 1024;
    static uint64 constexpr TIME_SLICE = 2;
};

#endif //OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP
