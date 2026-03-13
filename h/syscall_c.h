//   0x01 - mem_alloc          0x02 - mem_free
//   0x03 - mem_get_free_space 0x04 - mem_get_largest_free_block
//   0x11 - thread_create      0x12 - thread_exit      0x13 - thread_dispatch
//   0x21 - sem_open           0x22 - sem_close
//   0x23 - sem_wait           0x24 - sem_signal
//   0x31 - time_sleep
//   0x41 - getc               0x42 - putc

#ifndef _SYSCALL_C_H_
#define _SYSCALL_C_H_

#include "../lib/hw.h"

void* mem_alloc(size_t size);
int mem_free(void*);
size_t mem_get_free_space();
size_t mem_get_largest_free_block();

class _thread;
typedef _thread* thread_t;

int thread_create(thread_t* handle, void(*start_routine)(void*), void* arg);
int thread_exit();
void thread_dispatch();

class _sem;
typedef _sem* sem_t;

int sem_open(sem_t* handle, unsigned init);
int sem_close(sem_t handle);
int sem_wait(sem_t id);
int sem_signal(sem_t id);

int time_sleep(time_t);

const int EOF = -1;
char getc();
void putc(char);

#endif // _SYSCALL_C_H_
