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