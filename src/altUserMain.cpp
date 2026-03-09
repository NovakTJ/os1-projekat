#include "../h/syscall_c.h"
#include "../test/printing.hpp"

// Alternative to userMain() - simple semaphore test without getc.
// To use: change userMainWrapper in main.cpp to call altUserMain() instead of userMain().
// N producers each put their ID into a shared buffer, 1 consumer reads and prints them.

static const int BUF_CAP = 16;
static const int NUM_PRODUCERS = 3;
static const int ITEMS_PER_PRODUCER = 50;

// Circular buffer protected by semaphores
static int buf[BUF_CAP];
static int head = 0, tail = 0;

static sem_t itemAvailable;
static sem_t spaceAvailable;
static sem_t mutexHead;
static sem_t mutexTail;
static sem_t waitForAll;

static void bufPut(int val) {
    sem_wait(spaceAvailable);
    sem_wait(mutexTail);
    buf[tail] = val;
    tail = (tail + 1) % BUF_CAP;
    sem_signal(mutexTail);
    sem_signal(itemAvailable);
}

static int bufGet() {
    sem_wait(itemAvailable);
    sem_wait(mutexHead);
    int val = buf[head];
    head = (head + 1) % BUF_CAP;
    sem_signal(mutexHead);
    sem_signal(spaceAvailable);
    return val;
}

static void producer(void* arg) {
    int id = (int)(uint64)arg;
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        bufPut(id + '0');
        if (i % 10 == 0) thread_dispatch();
    }
    sem_signal(waitForAll);
}

static void consumer(void* arg) {
    (void)arg;
    int total = NUM_PRODUCERS * ITEMS_PER_PRODUCER;
    for (int i = 0; i < total; i++) {
        int ch = bufGet();
        putc((char)ch);
        if ((i + 1) % 60 == 0) putc('\n');
        if (i % 20 == 0) thread_dispatch();
    }
    putc('\n');
    sem_signal(waitForAll);
}

void altUserMain() {
    printString("=== Semaphore test (no getc) ===\n");
    printString("3 producers x 50 items, 1 consumer, buffer=16\n");

    sem_open(&itemAvailable, 0);
    sem_open(&spaceAvailable, BUF_CAP);
    sem_open(&mutexHead, 1);
    sem_open(&mutexTail, 1);
    sem_open(&waitForAll, 0);

    thread_t cons;
    thread_create(&cons, consumer, nullptr);

    thread_t prods[NUM_PRODUCERS];
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        thread_create(&prods[i], producer, (void*)(uint64)(i + 1));
    }

    // Wait for all producers + consumer
    for (int i = 0; i < NUM_PRODUCERS + 1; i++) {
        sem_wait(waitForAll);
    }

    sem_close(itemAvailable);
    sem_close(spaceAvailable);
    sem_close(mutexHead);
    sem_close(mutexTail);
    sem_close(waitForAll);

    printString("=== Semaphore test PASSED ===\n");
}
