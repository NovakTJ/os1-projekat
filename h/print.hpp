//
// Created by marko on 20.4.22..
//

#ifndef OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_PRINT_HPP
#define OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_PRINT_HPP

#include "../lib/hw.h"

// Kernel-mode (calls __putc directly, disables SIE for atomicity)
extern void printString(char const *string);
extern void printInteger(uint64 integer);
extern void printHexInteger(uint64 integer);

// User-mode (calls putc syscall)
// TODO: add mutex/semaphore protection once semaphores are working
extern void u_printString(char const *string);
extern void u_printInteger(uint64 integer);
extern void u_printHexInteger(uint64 integer);

#endif //OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_PRINT_HPP
