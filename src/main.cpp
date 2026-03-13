#include "../h/tcb.hpp"
#include "../h/riscv.hpp"
#include "../h/MemoryAllocator.hpp"
#include"../h/io.h"
#include"../h/_buffer.hpp"
#include"../h/syscall_c.h"
extern void userMain();
extern void altUserMain();
extern void testHeavyMemory123();
extern void testHeavyMemory4();

static volatile bool idleStop = false;
volatile bool oThreadStop = false;

static void userMainWrapper(void* arg) {
    (void)arg;



    userMain();
}

static void idleBody(void* arg) {
    (void)arg;
    while (!idleStop) {
        thread_dispatch();
    }
}

static void heavyMemory123Wrapper(void* arg) {
    (void)arg;
    testHeavyMemory123();
}

static void heavyMemory4Wrapper(void* arg) {
    (void)arg;
    testHeavyMemory4();
}
void finalMain(){
    _buf::initBuffers();

    TCB* boot = TCB::createForCurrent();
    TCB::running = boot;
	TCB* othread;
    TCB::createKernelThread(&othread, TCB::OThreadBody, nullptr);

    TCB* idleThread;
    TCB::createThread(&idleThread, idleBody, nullptr);

    TCB* userThread;
    TCB::createThread(&userThread, userMainWrapper, nullptr);

    Riscv::ms_sie(Riscv::SIE_SEIE);
    Riscv::ms_sstatus(Riscv::SSTATUS_SIE);

    while (!userThread->isFinished()) {
        TCB::kDispatch();
    }

    oThreadStop = true;
    idleStop = true;
    while (!othread->isFinished() || !idleThread->isFinished()) {
        TCB::kDispatch();
    }

	delete _buf::ob;
	delete _buf::ib;
    delete othread;
    delete idleThread;
    delete userThread;
    delete boot;
}

void nonPreemptiveTestMain(){
    TCB* boot = TCB::createForCurrent();
    TCB::running = boot;

    // Run tests 1-3 together
    TCB* test123;
    TCB::createNonPreemptive(&test123, heavyMemory123Wrapper, nullptr);

    while (!test123->isFinished()) {
        TCB::kDispatch();
    }
    delete test123;

    // Run test 4 separately (uses all memory, doesn't work after 1-3)
    TCB* test4;
    TCB::createNonPreemptive(&test4, heavyMemory4Wrapper, nullptr);

    while (!test4->isFinished()) {
        TCB::kDispatch();
    }
    delete test4;

    delete boot;
}

int main()
{
    MemoryAllocator::init();
    Riscv::w_stvec((uint64) &Riscv::supervisorTrap);

    finalMain();

    return 0;
}
