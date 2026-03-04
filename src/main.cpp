//
// Created by marko on 20.4.22..
//

#include "../h/tcb.hpp"
#include "../h/workers.hpp"
#include "../h/print.hpp"
#include "../h/riscv.hpp"


//TODO: delete
#include "../h/MemoryAllocator.h"
#include "../lib/console.h"

int main()
{
    __putc('I');
    __putc('n');
    __putc('i');
    __putc('t');
    __putc('\n');

    MemoryAllocator::init();    __putc('D');
    __putc('o');
    __putc('n');
    __putc('e');
    __putc('\n');

    char* ptr = MemoryAllocator::allocate(3);
    MemoryAllocator::deallocate(ptr);
    char* ptr2 = MemoryAllocator::allocate(3);
    ptr = MemoryAllocator::allocate(3);
    MemoryAllocator::deallocate(ptr);
    ptr = MemoryAllocator::allocate(3);
    MemoryAllocator::deallocate(ptr);
    MemoryAllocator::deallocate(ptr2);
    __putc('!');
    return 0;
    TCB *threads[5];

    threads[0] = TCB::createThread(nullptr);
    TCB::running = threads[0];

    threads[1] = TCB::createThread(workerBodyA);
    printString("ThreadA created\n");
    threads[2] = TCB::createThread(workerBodyB);
    printString("ThreadB created\n");
    threads[3] = TCB::createThread(workerBodyC);
    printString("ThreadC created\n");
    threads[4] = TCB::createThread(workerBodyD);
    printString("ThreadD created\n");

    Riscv::w_stvec((uint64) &Riscv::supervisorTrap); //konzola ce da sjebe ovo
    Riscv::ms_sstatus(Riscv::SSTATUS_SIE);

    while (!(threads[1]->isFinished() &&
             threads[2]->isFinished() &&
             threads[3]->isFinished() &&
             threads[4]->isFinished()))
    {
        TCB::yield();
    }

    for (auto &thread: threads)
    {
        delete thread;
    }
    printString("Finished\n");

    return 0;
}
