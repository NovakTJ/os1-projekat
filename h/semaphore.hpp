#ifndef _SEMAPHORE_HPP_
#define _SEMAPHORE_HPP_

#include "../lib/hw.h"
#include "MemoryAllocator.h"
#include "list.hpp"

class TCB;

class _sem {
public:
    void* operator new(size_t size) { return MemoryAllocator::allocate(size); }
    void operator delete(void* ptr) { MemoryAllocator::deallocate((char*)ptr); }

    _sem(unsigned init = 1);
    ~_sem();

    static int open(_sem** handle, unsigned init);
    static int close(_sem* handle);

    int wait();
    int signal();

private:
    int value;
    List<TCB> blockedQueue;
};

#endif // _SEMAPHORE_HPP_
