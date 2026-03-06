#include "../h/syscall_cpp.hpp"

// --- Thread ---

Thread::Thread(void (*body)(void*), void* arg)
    : myHandle(nullptr), body(body), arg(arg) {}

Thread::Thread()
    : myHandle(nullptr), body(nullptr), arg(nullptr) {}

Thread::~Thread() {}

int Thread::start() {
    if (body != nullptr) {
        return thread_create(&myHandle, body, arg);
    } else {
        //TODO: what then
    }
}

void Thread::dispatch() {
    thread_dispatch();
}

int Thread::sleep(time_t t) {
    return time_sleep(t);
}

// --- Console ---

char Console::getc() {
    return ::getc();
}

void Console::putc(char c) {
    ::putc(c);
}

// --- Semaphore ---

Semaphore::Semaphore(unsigned init) : myHandle(nullptr) {
    sem_open(&myHandle, init);
}

Semaphore::~Semaphore() {
    sem_close(myHandle);
}

int Semaphore::wait() {
    return sem_wait(myHandle);
}

int Semaphore::signal() {
    return sem_signal(myHandle);
}
