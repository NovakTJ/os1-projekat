//
// Created by os on 1/30/26.
//

#ifndef OS1_PROJEKAT_MEMORYALLOCATOR_H
#define OS1_PROJEKAT_MEMORYALLOCATOR_H
#include"../lib/hw.h"
#define MINTSIZE 32
#define MMDSIZE 64
struct MMD
{
    uint32 size;
    uint32 nextBID;
    MMD* next();
    MMD(uint32 s, MMD* n);
};
class MemoryAllocator
{
    public:
    static char* allocate(size_t nBlocks);
    static void deallocate(char* address);

    static char* startAddr;
    static char* endAddr;
    static MMD* iteratorAddress;

    static uint32 getBID(char* address);
    static char* getAdd(uint32 bid);
    static void init();
};

#endif //OS1_PROJEKAT_MEMORYALLOCATOR_H