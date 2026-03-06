//
// Created by marko on 20.4.22..
//

#include "../h/riscv.hpp"
#include "../h/tcb.hpp"
#include "../h/semaphore.hpp"
#include "../lib/console.h"
#include "../h/MemoryAllocator.h"
#include "../h/print.hpp"


//TODO: ctrl replace MemoryAllocator.h with .hpp everywhere and rename the file too
void Riscv::popSppSpie()
{
    __asm__ volatile("csrw sepc, ra"); //???
    __asm__ volatile("sret");
}
void Riscv::handleSupervisorTrap()
{
    uint64 scause = r_scause();
    if (scause == 0x0000000000000008UL || scause == 0x0000000000000009UL)
    {

        // interrupt: no; cause code: environment call from U-mode(8) or S-mode(9)

        bool hasReturnValue = true;
        uint64 volatile opcode;
        uint64 volatile arg1;
        uint64 volatile arg2;
        uint64 volatile arg3;
        uint64 volatile syscallReturnValue;
        __asm__ volatile("ld %0, 80(s0)" : "=r"(opcode)); //DO NOT PUT THIS IN ANOTHER FUNCTION
        __asm__ volatile("ld %0, 88(s0)" : "=r"(arg1)); //DO NOT PUT THIS IN ANOTHER FUNCTION
        __asm__ volatile("ld %0, 96(s0)" : "=r"(arg2)); //DO NOT PUT THIS IN ANOTHER FUNCTION
        __asm__ volatile("ld %0, 104(s0)" : "=r"(arg3)); //DO NOT PUT THIS IN ANOTHER FUNCTION

        switch (opcode)
        {
        case 0x01:
            syscallReturnValue = (uint64)MemoryAllocator::allocate(arg1);
            break;
        case 0x02:
            syscallReturnValue = (uint64)MemoryAllocator::deallocate((char*)arg1);
            break;
        case 0x03:
            syscallReturnValue = (uint64)MemoryAllocator::totalAvailableBytes();
            break;
        case 0x04:
            //TODO: implemet the following: syscallReturnValue = (uint64)MemoryAllocator::largestAvailableBlock();
            break;
        case 0x13:{
                uint64 volatile sepc = r_sepc();
                uint64 volatile sstatus = r_sstatus();
                TCB::timeSliceCounter = 0;
                TCB::urosDispatch(); //execution stops and later continues here, in contextSwitch.S
                w_sstatus(sstatus);
                w_sepc(sepc);
                hasReturnValue = false;
                break;
        }
        case 0x21: // sem_open
            syscallReturnValue = (uint64)_sem::open((_sem**)arg1, (unsigned)arg2);
            break;
        case 0x22: // sem_close
            syscallReturnValue = (uint64)_sem::close((_sem*)arg1);
            break;
        case 0x23: { // sem_wait (may block and context switch)
            uint64 volatile sepc = r_sepc();
            uint64 volatile sstatus = r_sstatus();
            syscallReturnValue = (uint64)((_sem*)arg1)->wait();
            w_sstatus(sstatus);
            w_sepc(sepc);
            break;
        }
        case 0x24: // sem_signal
            //TODO: may context switch too?
            syscallReturnValue = (uint64)((_sem*)arg1)->signal();
            break;
        default:
            hasReturnValue = false;
            __asm__ volatile("addi x1, x1, 0"); //noop
            printString("unexpected a0 (prob. 0), sepc=");
            printHexInteger(r_sepc());
            printString(" opcode=");
            printHexInteger(opcode);
            printString("\n");
            break;
        }
        w_sepc(r_sepc() + 4);
        if (hasReturnValue)
        {
            __asm__ volatile("sd %0, 80(s0)" : : "r"(syscallReturnValue)); //DO NOT PUT THIS IN ANOTHER FUNCTION
        }
    }
    else if (scause == 0x8000000000000001UL)
    {
        // interrupt: yes; cause code: supervisor software interrupt (CLINT; machine timer interrupt)
        mc_sip(SIP_SSIP);
        TCB::timeSliceCounter++;
        if (TCB::timeSliceCounter >= TCB::running->getTimeSlice())
        {
            uint64 volatile sepc = r_sepc();
            uint64 volatile sstatus = r_sstatus();
            TCB::timeSliceCounter = 0;
            TCB::urosDispatch();  //execution stops and later continues here, in contextSwitch.S
            w_sstatus(sstatus);
            w_sepc(sepc);
        }
    }
    else if (scause == 0x8000000000000009UL)
    {
        // interrupt: yes; cause code: supervisor external interrupt (PLIC; could be keyboard)
        console_handler();
    }
    else
    {
        printString("unexpected scause (prob. 2), sepc=");
        printHexInteger(r_sepc());
            printString("\n");

    }
}