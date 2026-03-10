#include "../h/syscall_c.h"
#include "../h/riscv.hpp"
#include "../h/MemoryAllocator.hpp"
// --- Memory ---

void* mem_alloc(size_t size) {
    size_t blocks = MemoryAllocator::neededBlocks(size);
    return (void*)Riscv::ecall(0x01, blocks);
}

int mem_free(void* ptr) {
    return (int)Riscv::ecall(0x02, (uint64)ptr);
}

size_t mem_get_free_space() {
    return (size_t)Riscv::ecall(0x03);
}

size_t mem_get_largest_free_block() {
    return (size_t)Riscv::ecall(0x04);
}

// --- Thread ---

int thread_create(thread_t* handle, void(*start_routine)(void*), void* arg) {
    // Allocate stack via mem_alloc, then pass last byte to ABI
    void* stack_mem = mem_alloc(DEFAULT_STACK_SIZE);
    if (!stack_mem && start_routine != nullptr) return -1;
    void* stack_top = start_routine != nullptr ? (void*)((uint64)stack_mem + DEFAULT_STACK_SIZE) : nullptr;
    return (int)Riscv::ecall(0x11, (uint64)handle, (uint64)start_routine, (uint64)arg, (uint64)stack_top);
}

int thread_exit() {
    return (int)Riscv::ecall(0x12);
}

void thread_dispatch() {
    Riscv::ecall(0x13);
}

// --- Time ---

int time_sleep(time_t t) {
    return (int)Riscv::ecall(0x31, (uint64)t);
}

// --- Console ---

char getc() {
    return (char)Riscv::ecall(0x41);
}

void putc(char c) {
    Riscv::ecall(0x42, (uint64)c);
}

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
