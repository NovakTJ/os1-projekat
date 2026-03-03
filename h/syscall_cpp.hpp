// ============================================================================
// syscall_cpp.hpp - C++ System Call API (OOP wrappers around C API)
// ============================================================================
// TODO: Implement all these methods in a corresponding .cpp file (e.g.
//       src/syscall_cpp.cpp). Each method should delegate to the
//       corresponding C API function from syscall_c.h.
// ============================================================================

#ifndef _SYSCALL_CPP_HPP_
#define _SYSCALL_CPP_HPP_

#include "syscall_c.h"

// Global operator new/delete - redirect to mem_alloc/mem_free syscalls
void* operator new(size_t);
void operator delete(void*);

// --- Thread ---
class Thread {
public:
    Thread(void (*body)(void*), void* arg);
    virtual ~Thread();

    int start();

    static void dispatch();
    static int sleep(time_t);

protected:
    Thread();
    virtual void run() {}

private:
    thread_t myHandle;
    void (*body)(void*);
    void* arg;
};

// --- Semaphore ---
class Semaphore {
public:
    Semaphore(unsigned init = 1);
    virtual ~Semaphore();

    int wait();
    int signal();

private:
    sem_t myHandle;
};

// --- PeriodicThread ---
class PeriodicThread : public Thread {
public:
    void terminate();

protected:
    PeriodicThread(time_t period);
    virtual void periodicActivation() {}

private:
    time_t period;
};

// --- Console ---
class Console {
public:
    static char getc();
    static void putc(char);
};

#endif // _SYSCALL_CPP_HPP_
