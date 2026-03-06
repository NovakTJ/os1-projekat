//
// Created by os on 3/5/26.
//

#include "../h/tcb.hpp"
#include"../h/print.hpp"
#include "../h/riscv.hpp"
#include "../h/workers.hpp"
void urosThreadTest()
{
TCB *threads[5];

TCB::createThread(threads, nullptr, (void*)0);
TCB::running = threads[0];

TCB::createThread(threads+1, workerBodyA, (void*)0);
printString("ThreadA created\n");
TCB::createThread(threads+2, workerBodyB, (void*)0);
printString("ThreadB created\n");
TCB::createThread(threads+3, workerBodyC, (void*)0);
printString("ThreadC created\n");
TCB::createThread(threads+4, workerBodyD, (void*)0);
printString("ThreadD created\n");

Riscv::ms_sstatus(Riscv::SSTATUS_SIE);
printHexInteger((uint64)threads[1]);
printString("\n");	printHexInteger((uint64)threads[2]);
printString("\n");	printHexInteger((uint64)threads[3]);
printString("\n");	printHexInteger((uint64)threads[4]);
printString("\n");
while (!(threads[1]->isFinished() &&
         threads[2]->isFinished() &&
         threads[3]->isFinished() &&
         threads[4]->isFinished()))
{
    TCB::yield();
}
printHexInteger((uint64)threads[1]);
printString("\n");	printHexInteger((uint64)threads[2]);
printString("\n");	printHexInteger((uint64)threads[3]);
printString("\n");	printHexInteger((uint64)threads[4]);
printString("\n");
for (auto &thread: threads)
{
    delete thread;
}
printString("Finished\n");

}