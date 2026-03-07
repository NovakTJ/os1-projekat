#include "../h/tcb.hpp"
#include "../h/riscv.hpp"
#include "../h/MemoryAllocator.h"

extern void userMain();
// extern void testHeavyMemory123();
// extern void testHeavyMemory4();

static void userMainWrapper(void* arg) {
    (void)arg;
    userMain();
}

// static void heavyMemory123Wrapper(void* arg) {
//     (void)arg;
//     testHeavyMemory123();
// }
//
// static void heavyMemory4Wrapper(void* arg) {
//     (void)arg;
//     testHeavyMemory4();
// }

void finalMain(){
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
}

void nonPreemptiveTestMain(){
    TCB* boot = TCB::createForCurrent();
    TCB::running = boot;

    // // Run tests 1-3 together
    // TCB* test123;
    // TCB::createNonPreemptive(&test123, heavyMemory123Wrapper, nullptr);
    //
    // while (!test123->isFinished()) {
    //     TCB::urosDispatch();
    // }
    // delete test123;
    //
    // // Run test 4 separately (uses all memory, doesn't work after 1-3)
    // TCB* test4;
    // TCB::createNonPreemptive(&test4, heavyMemory4Wrapper, nullptr);
    //
    // while (!test4->isFinished()) {
    //     TCB::urosDispatch();
    // }
    // delete test4;

    TCB* userThread;
    TCB::createNonPreemptive(&userThread, userMainWrapper, nullptr);

    while (!userThread->isFinished()) {
        TCB::urosDispatch();
    }
    delete userThread;

    delete boot;
}

int main()
{
    MemoryAllocator::init();
    Riscv::w_stvec((uint64) &Riscv::supervisorTrap);

    nonPreemptiveTestMain();

    return 0;
}
