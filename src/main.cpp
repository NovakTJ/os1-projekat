#include "../h/tcb.hpp"
#include "../h/riscv.hpp"
#include "../h/MemoryAllocator.h"

extern void userMain();

static void userMainWrapper(void* arg) {
    (void)arg; //if we want to do sth first but we dont need to. arg is nullptr in the current call.
    userMain();
}

int main()
{
    MemoryAllocator::init();
    Riscv::w_stvec((uint64) &Riscv::supervisorTrap);

    TCB* boot = TCB::createForCurrent();
    TCB::running = boot;

    TCB* userThread;
    TCB::createThread(&userThread, userMainWrapper, nullptr);

    Riscv::ms_sstatus(Riscv::SSTATUS_SIE);

    while (!userThread->isFinished()) {
        TCB::urosDispatch();
    }

    delete userThread;
    delete boot;

    return 0;
}
