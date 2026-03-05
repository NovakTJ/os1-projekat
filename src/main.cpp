//
// Created by marko on 20.4.22..
//

#include "../h/tcb.hpp"
#include "../h/workers.hpp"
#include "../h/print.hpp"
#include "../h/riscv.hpp"
#include "../h/MemoryAllocator.h"


//TODO: delete
#include "../h/MemoryAllocator.h"
#include "../lib/console.h"
#include "../lib/mem.h"

//TODO: skontati focus i escap eu clionu. valjda moze i bez toga. ima vise test primera sa escape prekidanjem.

int main()
{

    MemoryAllocator::init();
    Riscv::w_stvec((uint64) &Riscv::supervisorTrap); //konzola ce da sjebe ovo
    for (int bytes = 1; bytes<200; bytes++)
    {
        if (
            MemoryAllocator::neededBlocks(bytes) !=
            MemoryAllocator::neededBlocks(MemoryAllocator::neededBytes(MemoryAllocator::neededBlocks(bytes)))
            ) printString("mismatch!\n");
    }
     printString("no mismatch!\n");

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
