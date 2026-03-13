#ifndef _SEMAPHORE_HPP_
#define _SEMAPHORE_HPP_

#include "../lib/hw.h"
#include "MemoryAllocator.hpp"
#include "blockedQueue.hpp"

class TCB;

class _sem {
public:
    void* operator new(size_t size) { return MemoryAllocator::allocateBytes(size); }
    void operator delete(void* ptr) { MemoryAllocator::deallocate((char*)ptr); }

    _sem(unsigned init = 1);
    ~_sem();

    static int open(_sem** handle, unsigned init);
    static int close(_sem* handle);

    int wait();
    int signal();
    void print();
    static void printAll();

private:
    int putIntoQueue();

    bool works;
    int value;
    BlockedQueue blockedQueue;

    static const int MAX_SEMS = 256;
    static _sem* allSems[MAX_SEMS];
};

#endif // _SEMAPHORE_HPP_
