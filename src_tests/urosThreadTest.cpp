//
// Created by os on 3/5/26.
//

#include "../h/tcb.hpp"
#include "../h/syscall_c.h"
#include"../h/print.hpp"
#include "../h/riscv.hpp"
#include "../h/workers.hpp"
void urosThreadTest()
{
TCB *threads[5];

TCB::createThread(threads, nullptr, (void*)0);
TCB::running = threads[0];

TCB::createThread(threads+1, workerBodyA, (void*)0);
printKString("ThreadA created\n");
TCB::createThread(threads+2, workerBodyB, (void*)0);
printKString("ThreadB created\n");
TCB::createThread(threads+3, workerBodyC, (void*)0);
printKString("ThreadC created\n");
TCB::createThread(threads+4, workerBodyD, (void*)0);
printKString("ThreadD created\n");

Riscv::ms_sstatus(Riscv::SSTATUS_SIE);
printKHexInteger((uint64)threads[1]);
printKString("\n");	printKHexInteger((uint64)threads[2]);
printKString("\n");	printKHexInteger((uint64)threads[3]);
printKString("\n");	printKHexInteger((uint64)threads[4]);
printKString("\n");
while (!(threads[1]->isFinished() &&
         threads[2]->isFinished() &&
         threads[3]->isFinished() &&
         threads[4]->isFinished()))
{
    thread_dispatch();
}
printKHexInteger((uint64)threads[1]);
printKString("\n");	printKHexInteger((uint64)threads[2]);
printKString("\n");	printKHexInteger((uint64)threads[3]);
printKString("\n");	printKHexInteger((uint64)threads[4]);
printKString("\n");
for (auto &thread: threads)
{
    delete thread;
}
printKString("Finished\n");

}