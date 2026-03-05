//
// Created by os on 1/30/26.
//

#ifndef OS1_PROJEKAT_MEMORYALLOCATOR_H
#define OS1_PROJEKAT_MEMORYALLOCATOR_H
#include"../lib/hw.h"



struct MMD
{
    uint32 size; //number of blocks, including the next metadata.
    uint32 nextBID;
    MMD* getNext(); //nextAddr()
    void setNext(MMD* a); //nextAddr()
    MMD(uint32 s, MMD* n);
    bool isAllocated();
    void setAllocated(bool a);
};
class MemoryAllocator
{
    public:
    static bool initialized;
    static char* allocateBytes(size_t nBytes);
    static char* allocateBlocks(size_t nBlocks);
    static int deallocate(char* address);

    static char* startAddr;
    static char* endAddr;
    static MMD* iteratorAddress;
    static uint32 totalAvailableBlocks;
    static size_t totalAvailableBytes();

    static uint32 getBID(char* address);
    static char* getAdd(uint32 bid);
    static size_t MMDSIZE;
    static void init();
    static void printInfo();

    static size_t neededBytes(size_t nBlocks);
    static uint32 neededBlocks(size_t nBytes);
private:
    //terminology:
    //slab is the name for the contiguous space of memory that is one element in the list
    static bool canMerge(MMD* mmd);
    static void merge(MMD* mmd);
    static bool slabGood(MMD* mmd, size_t size);
    static bool slabMoreThanGood(MMD* mmd, size_t size);
    static MMD* MaybeCreateAndReturnNextMMD(MMD* mmd, size_t nBlocks);
    static MMD* createFirstTwoSlabs();
    static uint32 listSize;

};

#endif //OS1_PROJEKAT_MEMORYALLOCATOR_H