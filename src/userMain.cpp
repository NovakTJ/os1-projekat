#include "../h/print.hpp"
#include "../h/syscall_c.h"

extern void urosUModeThreadTest();

void userMain() {
    urosUModeThreadTest();
}
