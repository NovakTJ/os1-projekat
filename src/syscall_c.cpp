#include "../h/syscall_c.h"
#include "../h/riscv.hpp"

// --- Semaphore ---

int sem_open(sem_t* handle, unsigned init) {
    return (int)Riscv::ecall(0x21, (uint64)handle, (uint64)init);
}

int sem_close(sem_t handle) {
    return (int)Riscv::ecall(0x22, (uint64)handle);
}

int sem_wait(sem_t id) {
    return (int)Riscv::ecall(0x23, (uint64)id);
}

int sem_signal(sem_t id) {
    return (int)Riscv::ecall(0x24, (uint64)id);
}
