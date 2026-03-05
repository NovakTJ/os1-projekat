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
#include "../lib/mem.h"
extern void urosThreadTest();
int main()
{

    MemoryAllocator::init();
    Riscv::w_stvec((uint64) &Riscv::supervisorTrap); //konzola ce da sjebe ovo
	urosThreadTest();
    return 0;
}
