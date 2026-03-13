//
// Created by marko on 20.4.22..
//

#ifndef OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_PRINT_HPP
#define OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_PRINT_HPP

#include "../lib/hw.h"

extern void printKString(char const *string);
extern void printKInteger(uint64 integer);
extern void printKHexInteger(uint64 integer);

extern void printAllThreads();

extern void printUString(char const *string);
extern void printUInteger(uint64 integer);
extern void printUHexInteger(uint64 integer);

#endif //OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_PRINT_HPP
