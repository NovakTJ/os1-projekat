#include "../h/print.hpp"
#include "../h/syscall_c.h"

void userMain() {
    u_printString("Hello from user mode!\n");
    u_printInteger(42);
    u_printString("\n");
    u_printHexInteger(0xDEAD);
    u_printString("\n");
    u_printString("u_print test done.\n");
}
