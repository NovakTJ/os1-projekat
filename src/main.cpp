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
extern void urosThreadTest();
extern void testHeavyMemory();

int main()
{

    MemoryAllocator::init();
    Riscv::w_stvec((uint64) &Riscv::supervisorTrap); //konzola ce da sjebe ovo
//testHeavyMemory();
//TODO: call testHeavyMemory from a U-mode thread. im getting errors because of nested ecalls. maybe work on the final usermain call and call the test from usermain.
urosThreadTest();

    return 0;
}
