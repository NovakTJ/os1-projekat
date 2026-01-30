#include"../h/MemoryAllocator.h"

// Static member definitions
char* MemoryAllocator::startAddr = nullptr;
char* MemoryAllocator::endAddr = nullptr;
MMD* MemoryAllocator::iteratorAddress = nullptr;

// MMD member function implementations
MMD* MMD::next()
{
    return (MMD*)MemoryAllocator::getAdd(nextBID);
}

MMD::MMD(uint32 s, MMD* n)
{
    size = s;
    nextBID = MemoryAllocator::getBID(n);
}

// MemoryAllocator member function implementations
uint32 MemoryAllocator::getBID(char* address)
{
    return (address - startAddr) / MEM_BLOCK_SIZE;
}

char* MemoryAllocator::getAdd(uint32 bid)
{
    return startAddr + bid * MEM_BLOCK_SIZE;
}

void MemoryAllocator::init()
{
    startAddr = (char*)(((size_t)HEAP_START_ADDR + MMDSIZE) / MEM_BLOCK_SIZE + 1) * MEM_BLOCK_SIZE;
    endAddr = (char*)((size_t)HEAP_END_ADDR / MEM_BLOCK_SIZE) * MEM_BLOCK_SIZE;
    iteratorAddress = (MMD*)(startAddr - MMDSIZE);

    *iteratorAddress = MMD(
        (endAddr - startAddr) / MEM_BLOCK_SIZE,
        iteratorAddress
    );
}

char* MemoryAllocator::allocate(size_t nBlocks)
{
    // TODO: Implement allocation logic
    return nullptr;
}

void MemoryAllocator::deallocate(char* address)
{
    // TODO: Implement deallocation logic
}