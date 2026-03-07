//
// Created by os on 3/7/26.
// User-mode thread test - same as urosThreadTest but uses syscall APIs
//

#include "../h/syscall_c.h"
#include "../h/print.hpp"

static volatile int finishedCount;
static volatile int exitTestPassed;

// thread_exit test: prints 0-2 then exits early, should never print 3+
static void u_workerExit(void* arg) {
    for (uint64 i = 0; i < 6; i++) {
        if (i == 3) {
            u_printString("E: calling thread_exit at i=3\n");
            exitTestPassed = 1;
            thread_exit();
            // should never reach here
            u_printString("E: ERROR - thread_exit did not terminate thread!\n");
            exitTestPassed = 0;
        }
        u_printString("E: i=");
        u_printInteger(i);
        u_printString("\n");
    }
    exitTestPassed = 0; // should never reach here either
}

static void u_workerA(void* arg) {
    for (uint64 i = 0; i < 10; i++) {
        u_printString("A: i=");
        u_printInteger(i);
        u_printString("\n");
        for (uint64 j = 0; j < 10000; j++)
            for (uint64 k = 0; k < 30000; k++) {}
    }
    finishedCount++;
}

static void u_workerB(void* arg) {
    for (uint64 i = 0; i < 16; i++) {
        u_printString("B: i=");
        u_printInteger(i);
        u_printString("\n");
        for (uint64 j = 0; j < 10000; j++)
            for (uint64 k = 0; k < 30000; k++) {}
    }
    finishedCount++;
}

static uint64 fibonacci(uint64 n) {
    if (n == 0 || n == 1) return n;
    if (n % 10 == 0) thread_dispatch();
    return fibonacci(n - 1) + fibonacci(n - 2);
}

static void u_workerC(void* arg) {
    uint8 i = 0;
    for (; i < 3; i++) {
        u_printString("C: i=");
        u_printInteger(i);
        u_printString("\n");
    }

    u_printString("C: yield\n");
    __asm__ ("li t1, 7");
    thread_dispatch();

    uint64 t1 = 0;
    __asm__ ("mv %[t1], t1" : [t1] "=r"(t1));

    u_printString("C: t1=");
    u_printInteger(t1);
    u_printString("\n");

    uint64 result = fibonacci(12);
    u_printString("C: fibonaci=");
    u_printInteger(result);
    u_printString("\n");

    for (; i < 6; i++) {
        u_printString("C: i=");
        u_printInteger(i);
        u_printString("\n");
    }
    finishedCount++;
}

static void u_workerD(void* arg) {
    uint8 i = 10;
    for (; i < 13; i++) {
        u_printString("D: i=");
        u_printInteger(i);
        u_printString("\n");
    }

    u_printString("D: yield\n");
    __asm__ ("li t1, 5");
    thread_dispatch();

    uint64 result = fibonacci(16);
    u_printString("D: fibonaci=");
    u_printInteger(result);
    u_printString("\n");

    for (; i < 16; i++) {
        u_printString("D: i=");
        u_printInteger(i);
        u_printString("\n");
    }
    finishedCount++;
}

void urosUModeThreadTest() {
    // --- thread_exit test ---
    u_printString("=== thread_exit test ===\n");
    exitTestPassed = 0;
    thread_t exitThread;
    thread_create(&exitThread, u_workerExit, nullptr);
    // wait for it (it won't increment finishedCount, but it will set exitTestPassed)
    while (!exitTestPassed) {
        thread_dispatch();
    }
    // give it a couple more dispatches to make sure it didn't continue
    thread_dispatch();
    thread_dispatch();
    if (exitTestPassed == 1)
        u_printString("thread_exit: PASSED\n");
    else
        u_printString("thread_exit: FAILED\n");

    // --- mem_alloc / mem_free test ---
    u_printString("=== mem_alloc/mem_free test ===\n");
    size_t freeBefore = mem_get_free_space();
    u_printString("Free before alloc: ");
    u_printInteger(freeBefore);
    u_printString("\n");

    void* block = mem_alloc(1024);
    if (block) {
        u_printString("Allocated 1024 bytes at ");
        u_printHexInteger((uint64)block);
        u_printString("\n");

        // write and read back to verify the memory works
        char* p = (char*)block;
        p[0] = 'O'; p[1] = 'K';
        if (p[0] == 'O' && p[1] == 'K')
            u_printString("mem read/write: PASSED\n");
        else
            u_printString("mem read/write: FAILED\n");

        size_t freeAfter = mem_get_free_space();
        u_printString("Free after alloc: ");
        u_printInteger(freeAfter);
        u_printString("\n");

        int freeResult = mem_free(block);
        size_t freeAfterFree = mem_get_free_space();
        u_printString("Free after free:  ");
        u_printInteger(freeAfterFree);
        u_printString("\n");

        if (freeResult == 0 && freeAfterFree >= freeBefore)
            u_printString("mem_alloc/mem_free: PASSED\n");
        else
            u_printString("mem_alloc/mem_free: FAILED\n");
    } else {
        u_printString("mem_alloc: FAILED (returned null)\n");
    }

    // --- thread test (A-D) ---
    u_printString("=== thread create/dispatch test ===\n");
    finishedCount = 0;

    thread_t threads[4];

    thread_create(&threads[0], u_workerA, nullptr);
    u_printString("ThreadA created\n");
    thread_create(&threads[1], u_workerB, nullptr);
    u_printString("ThreadB created\n");
    thread_create(&threads[2], u_workerC, nullptr);
    u_printString("ThreadC created\n");
    thread_create(&threads[3], u_workerD, nullptr);
    u_printString("ThreadD created\n");

    while (finishedCount < 4) {
        thread_dispatch();
    }

    u_printString("=== all tests done ===\n");
}
