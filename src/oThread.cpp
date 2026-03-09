//
// Created by os on 3/7/26.
//
#include "../h/tcb.hpp"

void TCB::OThreadBody(void* arg)
{
    //no need for locks here

    //poll for the status bit in TX, put stuff in it if able, then (some stuff + kDispatch())? check the truth just in case

    //as for the input:
    //theres a piece of code in riscv.cpp for putting a char into the inputbuffer. signal() there.
    //getc syscalls will wait for that semaphore! wait() is inside riscv.
    //probably need an inputSemaphore file and this is an output thread instead.
}