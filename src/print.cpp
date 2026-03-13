//
// Created by marko on 20.4.22..
//

#include "../h/print.hpp"
#include "../h/io.h"
#include "../h/riscv.hpp"
#include "../h/syscall_c.h"
#include "../h/scheduler.hpp"
#include "../h/semaphore.hpp"
#include "../lib/console.h"

void printKString(char const *string)
{
    uint64 sstatus = Riscv::r_sstatus();
    Riscv::mc_sstatus(Riscv::SSTATUS_SIE);
    while (*string != '\0')
    {
        handlePutc(*string);
        string++;
    }
    Riscv::ms_sstatus(sstatus & Riscv::SSTATUS_SIE ? Riscv::SSTATUS_SIE : 0);
}

void printKInteger(uint64 integer)
{
    uint64 sstatus = Riscv::r_sstatus();
    Riscv::mc_sstatus(Riscv::SSTATUS_SIE);
    static char digits[] = "0123456789";
    char buf[16];
    int i, neg;
    uint x;

    neg = 0;
    if (integer < 0)
    {
        neg = 1;
        x = -integer;
    } else
    {
        x = integer;
    }

    i = 0;
    do
    {
        buf[i++] = digits[x % 10];
    } while ((x /= 10) != 0);
    if (neg)
        buf[i++] = '-';

    while (--i >= 0) { handlePutc(buf[i]); }
    Riscv::ms_sstatus(sstatus & Riscv::SSTATUS_SIE ? Riscv::SSTATUS_SIE : 0);
}

void printKHexInteger(uint64 integer)
{
    uint64 sstatus = Riscv::r_sstatus();
    Riscv::mc_sstatus(Riscv::SSTATUS_SIE);
    static char digits[] = "0123456789abcdef";
    char buf[16];
    int i = 0;

    if (integer == 0)
    {
        handlePutc('0');
        handlePutc('x');
        handlePutc('0');
        Riscv::ms_sstatus(sstatus & Riscv::SSTATUS_SIE ? Riscv::SSTATUS_SIE : 0);
        return;
    }

    uint64 x = integer;
    do
    {
        buf[i++] = digits[x & 0xf];
    } while ((x >>= 4) != 0);

    handlePutc('0');
    handlePutc('x');
    while (--i >= 0) { handlePutc(buf[i]); }
    Riscv::ms_sstatus(sstatus & Riscv::SSTATUS_SIE ? Riscv::SSTATUS_SIE : 0);
}


void printUString(char const *string)
{
    while (*string != '\0')
    {
        putc(*string);
        string++;
    }
}

void printUInteger(uint64 integer)
{
    static char digits[] = "0123456789";
    char buf[16];
    int i, neg;
    uint x;

    neg = 0;
    if (integer < 0)
    {
        neg = 1;
        x = -integer;
    } else
    {
        x = integer;
    }

    i = 0;
    do
    {
        buf[i++] = digits[x % 10];
    } while ((x /= 10) != 0);
    if (neg)
        buf[i++] = '-';

    while (--i >= 0) { putc(buf[i]); }
}

void printUHexInteger(uint64 integer)
{
    static char digits[] = "0123456789abcdef";
    char buf[16];
    int i = 0;

    if (integer == 0)
    {
        putc('0');
        putc('x');
        putc('0');
        return;
    }

    uint64 x = integer;
    do
    {
        buf[i++] = digits[x & 0xf];
    } while ((x >>= 4) != 0);

    putc('0');
    putc('x');
    while (--i >= 0) { putc(buf[i]); }
}

void printAllThreads()
{
    _sem::printAll();
    Scheduler::print();
    SleepingQueue::print();
}