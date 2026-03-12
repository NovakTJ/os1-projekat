//
// Created by marko on 20.4.22..
//

#include "../h/riscv.hpp"
#include "../h/tcb.hpp"
#include "../h/semaphore.hpp"
#include "../lib/console.h"
#include "../h/MemoryAllocator.hpp"
#include "../h/print.hpp"
#include "../h/io.h"

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
        uint64 volatile arg4;
        uint64 volatile syscallReturnValue;
        __asm__ volatile("ld %0, 80(s0)" : "=r"(opcode)); //DO NOT PUT THIS IN ANOTHER FUNCTION
        __asm__ volatile("ld %0, 88(s0)" : "=r"(arg1)); //DO NOT PUT THIS IN ANOTHER FUNCTION
        __asm__ volatile("ld %0, 96(s0)" : "=r"(arg2)); //DO NOT PUT THIS IN ANOTHER FUNCTION
        __asm__ volatile("ld %0, 104(s0)" : "=r"(arg3)); //DO NOT PUT THIS IN ANOTHER FUNCTION
        __asm__ volatile("ld %0, 112(s0)" : "=r"(arg4)); //DO NOT PUT THIS IN ANOTHER FUNCTION

        switch (opcode)
        {
        case 0x01:
            syscallReturnValue = (uint64)MemoryAllocator::allocateBlocks(arg1);
            break;
        case 0x02:
            syscallReturnValue = (uint64)MemoryAllocator::deallocate((char*)arg1);
            break;
        case 0x03:
            syscallReturnValue = (uint64)MemoryAllocator::totalAvailableBytes();
            break;
        case 0x04:
            syscallReturnValue = (uint64)MemoryAllocator::largestAvailableBlock();
            break;
        case 0x11: { // thread_create(handle, start_routine, arg, stack_space)
            syscallReturnValue = (uint64)TCB::createThread(
                (TCB**)arg1,
                (TCB::BodyWithArg)arg2,
                (void*)arg3,
                (void*)arg4
            );
            break;
        }
        case 0x12: { // thread_exit
            TCB::running->setFinished(true);
            TCB::kDispatch();
            printKString("massive error: dead thread walks again\n");
            hasReturnValue = false;
            break;
        }
        case 0x13:{
                TCB::timeSliceCounter = 0;
                TCB::kDispatch(); //execution stops and later continues here, in contextSwitch.S
                hasReturnValue = false;
                break;
        }
        case 0x41: { // getc (may block and context switch)
            syscallReturnValue = (uint64)handleGetc();
            break;
        }
        case 0x42: // putc
            handlePutc((char)arg1);
            hasReturnValue = false;
            break;
        case 0x21: // sem_open
            syscallReturnValue = (uint64)_sem::open((_sem**)arg1, (unsigned)arg2);
            break;
        case 0x22: // sem_close
            syscallReturnValue = (uint64)_sem::close((_sem*)arg1);
            break;
        case 0x23: { // sem_wait (may block and context switch)
            syscallReturnValue = (uint64)((_sem*)arg1)->wait();
            break;
        }
        case 0x24: // sem_signal
            syscallReturnValue = (uint64)((_sem*)arg1)->signal();
            break;
		case 0x31: //sleep
			syscallReturnValue = TCB::putCurrentToSleep(arg1);
			break;

        default:
            hasReturnValue = false;
            __asm__ volatile("addi x1, x1, 0"); //noop
            printKString("unexpected a0 (prob. 0), sepc=");
            printKHexInteger(r_sepc());
            printKString(" opcode=");
            printKHexInteger(opcode);
            printKString("\n");
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
        SleepingQueue::decrement();
        TCB::timeSliceCounter++;
        if (TCB::timeSliceCounter >= TCB::running->getTimeSlice())
        {
            TCB::timeSliceCounter = 0;
            TCB::kDispatch();  //execution stops and later continues here, in contextSwitch.S
        }
    }
    else if (scause == 0x8000000000000009UL)
    {
        // interrupt: yes; cause code: supervisor external interrupt (PLIC; could be keyboard)
        //console_handler();
        //append to inputbuffer, signal() to input semaphore.
        int irq = plic_claim();
         if (irq == CONSOLE_IRQ) {
             while (*((volatile uint8*)CONSOLE_STATUS) & CONSOLE_RX_STATUS_BIT) {
                 char c = *((volatile char*)CONSOLE_RX_DATA);
                 putIntoInputBuffer(c);
             }
         }
        plic_complete(irq);
    }
    else
    {
        printKString("unexpected scause=");
        printKHexInteger(scause);
        printKString(", sepc=");
        printKHexInteger(r_sepc());
        printKString("\n");

    }
}