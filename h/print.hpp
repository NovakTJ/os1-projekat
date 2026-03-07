//
// Created by marko on 20.4.22..
//

#ifndef OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_PRINT_HPP
#define OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_PRINT_HPP

#include "../lib/hw.h"

// Kernel-mode (calls __putc directly, disables SIE for atomicity)
extern void printKString(char const *string);
extern void printKInteger(uint64 integer);
extern void printKHexInteger(uint64 integer);

// User-mode (calls putc syscall)
// TODO: add mutex/semaphore protection once semaphores are working
extern void printUString(char const *string);
extern void printUInteger(uint64 integer);
extern void printUHexInteger(uint64 integer);

#endif //OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_PRINT_HPP
