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
            printUString("E: calling thread_exit at i=3\n");
            exitTestPassed = 1;
            thread_exit();
            // should never reach here
            printUString("E: ERROR - thread_exit did not terminate thread!\n");
            exitTestPassed = 0;
        }
        printUString("E: i=");
        printUInteger(i);
        printUString("\n");
    }
    exitTestPassed = 0; // should never reach here either
}

static void u_workerA(void* arg) {
    for (uint64 i = 0; i < 10; i++) {
        printUString("A: i=");
        printUInteger(i);
        printUString("\n");
        for (uint64 j = 0; j < 10000; j++)
            for (uint64 k = 0; k < 30000; k++) {}
    }
    finishedCount++;
}

static void u_workerB(void* arg) {
    for (uint64 i = 0; i < 16; i++) {
        printUString("B: i=");
        printUInteger(i);
        printUString("\n");
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
        printUString("C: i=");
        printUInteger(i);
        printUString("\n");
    }

    printUString("C: yield\n");
    __asm__ ("li t1, 7");
    thread_dispatch();

    uint64 t1 = 0;
    __asm__ ("mv %[t1], t1" : [t1] "=r"(t1));

    printUString("C: t1=");
    printUInteger(t1);
    printUString("\n");

    uint64 result = fibonacci(12);
    printUString("C: fibonaci=");
    printUInteger(result);
    printUString("\n");

    for (; i < 6; i++) {
        printUString("C: i=");
        printUInteger(i);
        printUString("\n");
    }
    finishedCount++;
}

static void u_workerD(void* arg) {
    uint8 i = 10;
    for (; i < 13; i++) {
        printUString("D: i=");
        printUInteger(i);
        printUString("\n");
    }

    printUString("D: yield\n");
    __asm__ ("li t1, 5");
    thread_dispatch();

    uint64 result = fibonacci(16);
    printUString("D: fibonaci=");
    printUInteger(result);
    printUString("\n");

    for (; i < 16; i++) {
        printUString("D: i=");
        printUInteger(i);
        printUString("\n");
    }
    finishedCount++;
}

void urosUModeThreadTest() {
    // --- thread_exit test ---
    printUString("=== thread_exit test ===\n");
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
        printUString("thread_exit: PASSED\n");
    else
        printUString("thread_exit: FAILED\n");

    // --- mem_alloc / mem_free test ---
    printUString("=== mem_alloc/mem_free test ===\n");
    size_t freeBefore = mem_get_free_space();
    printUString("Free before alloc: ");
    printUInteger(freeBefore);
    printUString("\n");

    void* block = mem_alloc(1024);
    if (block) {
        printUString("Allocated 1024 bytes at ");
        printUHexInteger((uint64)block);
        printUString("\n");

        // write and read back to verify the memory works
        char* p = (char*)block;
        p[0] = 'O'; p[1] = 'K';
        if (p[0] == 'O' && p[1] == 'K')
            printUString("mem read/write: PASSED\n");
        else
            printUString("mem read/write: FAILED\n");

        size_t freeAfter = mem_get_free_space();
        printUString("Free after alloc: ");
        printUInteger(freeAfter);
        printUString("\n");

        int freeResult = mem_free(block);
        size_t freeAfterFree = mem_get_free_space();
        printUString("Free after free:  ");
        printUInteger(freeAfterFree);
        printUString("\n");

        if (freeResult == 0 && freeAfterFree >= freeBefore)
            printUString("mem_alloc/mem_free: PASSED\n");
        else
            printUString("mem_alloc/mem_free: FAILED\n");
    } else {
        printUString("mem_alloc: FAILED (returned null)\n");
    }

    // --- thread test (A-D) ---
    printUString("=== thread create/dispatch test ===\n");
    finishedCount = 0;

    thread_t threads[4];

    thread_create(&threads[0], u_workerA, nullptr);
    printUString("ThreadA created\n");
    thread_create(&threads[1], u_workerB, nullptr);
    printUString("ThreadB created\n");
    thread_create(&threads[2], u_workerC, nullptr);
    printUString("ThreadC created\n");
    thread_create(&threads[3], u_workerD, nullptr);
    printUString("ThreadD created\n");

    while (finishedCount < 4) {
        thread_dispatch();
    }

    printUString("=== all tests done ===\n");
}
