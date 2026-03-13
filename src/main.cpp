#include "../h/tcb.hpp"
#include "../h/riscv.hpp"
#include "../h/MemoryAllocator.hpp"
#include"../h/io.h"
#include"../h/_buffer.hpp"
#include"../h/syscall_c.h"
#include"../h/print.hpp"
extern void userMain();
extern void altUserMain();
extern void testHeavyMemory123();
extern void testHeavyMemory4();

static volatile bool idleStop = false;
volatile bool oThreadStop = false;
int debugMode = 0;

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
	if(debugMode){
		printKHexInteger((uint64)boot);
		printKString(" is boot\n");
		printKHexInteger((uint64)othread);
		printKString(" is othread\n");
		printKHexInteger((uint64)idleThread);
		printKString(" is idleThread\n");
		printKHexInteger((uint64)userThread);
		printKString(" is userThread\n");
    	printAllThreads();
	}
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


int main()
{
    MemoryAllocator::init();
    Riscv::w_stvec((uint64) &Riscv::supervisorTrap);

    finalMain();

    return 0;
}
