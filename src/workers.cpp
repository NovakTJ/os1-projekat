//
// Created by marko on 20.4.22..
//

#include "../lib/hw.h"
#include "../h/tcb.hpp"
#include "../h/print.hpp"
#include "../h/syscall_c.h"
void workerBodyA(void* aa)
{
//deprecated tests, idk if they work still.
    for (uint64 i = 0; i < 10; i++)
    {
        printKString("A: i=");
        printKInteger(i);
        printKString("\n");
        for (uint64 j = 0; j < 10000; j++)
        {
            for (uint64 k = 0; k < 30000; k++)
            {
                // busy wait
            }
//            thread_dispatch();
        }
    }
}

void workerBodyB(void* aa)
{
    for (uint64 i = 0; i < 16; i++)
    {
        printKString("B: i=");
        printKInteger(i);
        printKString("\n");
        for (uint64 j = 0; j < 10000; j++)
        {
            for (uint64 k = 0; k < 30000; k++)
            {
                // busy wait
            }
//            thread_dispatch();
        }
    }
}

static uint64 fibonacci(uint64 n)
{
    if (n == 0 || n == 1) { return n; }
    if (n % 10 == 0) { thread_dispatch(); }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void workerBodyC(void* aa)
{
    uint8 i = 0;
    for (; i < 3; i++)
    {
        printKString("C: i=");
        printKInteger(i);
        printKString("\n");
    }

    printKString("C: yield\n");
    __asm__ ("li t1, 7");
    thread_dispatch();

    uint64 t1 = 0;
    __asm__ ("mv %[t1], t1" : [t1] "=r"(t1));

    printKString("C: t1=");
    printKInteger(t1);
    printKString("\n");

    uint64 result = fibonacci(12);
    printKString("C: fibonaci=");
    printKInteger(result);
    printKString("\n");

    for (; i < 6; i++)
    {
        printKString("C: i=");
        printKInteger(i);
        printKString("\n");
    }
//    thread_dispatch();
}

void workerBodyD(void* aa)
{
    uint8 i = 10;
    for (; i < 13; i++)
    {
        printKString("D: i=");
        printKInteger(i);
        printKString("\n");
    }

    printKString("D: yield\n");
    __asm__ ("li t1, 5");
    thread_dispatch();

    uint64 result = fibonacci(16);
    printKString("D: fibonaci=");
    printKInteger(result);
    printKString("\n");

    for (; i < 16; i++)
    {
        printKString("D: i=");
        printKInteger(i);
        printKString("\n");
    }
//    thread_dispatch();
}