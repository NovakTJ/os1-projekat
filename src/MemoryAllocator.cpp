#include"../h/MemoryAllocator.h"
#include"../h/print.hpp"
#include"../lib/mem.h"

// Static member definitions
char* MemoryAllocator::startAddr = nullptr;
char* MemoryAllocator::endAddr = nullptr;
MMD* MemoryAllocator::iteratorAddress = nullptr;
bool MemoryAllocator::initialized = false;
uint32 MemoryAllocator::totalAvailableBlocks = 0;
uint32 MemoryAllocator::listSize = 0;

size_t MemoryAllocator::totalAvailableBytes()
{
    return totalAvailableBlocks*MEM_BLOCK_SIZE-MMDSIZE;
}

size_t MemoryAllocator::MMDSIZE = 8;

void* __mem_alloc(size_t size)
{
    //TODO: this is not a system call and it should be. call ::allocate from stvec. here just li a0 and ecall.
    return (void*)MemoryAllocator::allocate(size);
}

int __mem_free(void* ptr)
{
    //TODO: this is not a system call and it should be. call ::deallocate from stvec. here just li a0 and ecall.
    return MemoryAllocator::deallocate((char*)ptr);
}


// MMD member function implementations
MMD* MMD::getNext()
{
    return (MMD*)MemoryAllocator::getAdd(nextBID) - 1;
}

void MMD::setNext(MMD* a)
{
    nextBID = MemoryAllocator::getBID((char*)(a+1));
}

MMD::MMD(uint32 s, MMD* n)
{
    size = s;
    setNext(n);
}

bool MMD::isAllocated()
{
    return nextBID == (uint32)(-1);
}

void MMD::setAllocated(bool a)
{
    if (a)
    {
        nextBID = (uint32)(-1);
    }
    else
    {
        nextBID = (uint32)(-2);
        //this should be updated immediately;
    }
}

uint32 MemoryAllocator::getBID(char* address)
{
    //the BID is short for block index.
    return (address - startAddr) / MEM_BLOCK_SIZE;
}

char* MemoryAllocator::getAdd(uint32 bid)
{
    //get address in char*
    return startAddr + bid * MEM_BLOCK_SIZE;
}
void MemoryAllocator::init()
{
    startAddr = (char*)((((size_t)HEAP_START_ADDR +MMDSIZE) / MEM_BLOCK_SIZE + 1) * MEM_BLOCK_SIZE);
    //safe if HEAP_START_ADDR is not divisible by block size.
    //also leaves space for one MMD.
    endAddr = (char*)((size_t)HEAP_END_ADDR / MEM_BLOCK_SIZE * MEM_BLOCK_SIZE);

    iteratorAddress = createFirstTwoSlabs();

    // totalAvailableBlocks initialized in createFirstTwoSlabs
    listSize = 2;
    initialized=true;
}

bool MemoryAllocator::canMerge(MMD* mmd)
{
    if (mmd->size == 0) return false;
    char* aboveAddr = (char*)mmd + mmd->size * MEM_BLOCK_SIZE;
    if (aboveAddr >= endAddr) return false;
    MMD* aboveMmd = (MMD*)aboveAddr;
    if (aboveMmd->size == 0) return false;
    return !aboveMmd->isAllocated();
}

void MemoryAllocator::merge(MMD* mmd)
{
    MMD* aboveMmd = (MMD*)((char*)mmd + mmd->size * MEM_BLOCK_SIZE);
    // Remove aboveMmd from the free list by finding its predecessor
    MMD* prev = mmd;
    while (prev->getNext() != aboveMmd)
    {
        prev = prev->getNext();
    }
    prev->setNext(aboveMmd->getNext());
    // Absorb aboveMmd's blocks into mmd
    mmd->size += aboveMmd->size;
    listSize--;
}

uint32 neededBlocks(size_t nBytes)
{
    size_t remainder = nBytes % MEM_BLOCK_SIZE;
    uint32 divResult = nBytes / MEM_BLOCK_SIZE;
    if (remainder <= MEM_BLOCK_SIZE-MemoryAllocator::MMDSIZE)
    {
        return divResult + 1;
    }
    else
    {
        return divResult + 2;
    }

}
bool MemoryAllocator::slabGood(MMD* mmd, size_t nBytes)
{
    return neededBlocks(nBytes)<=mmd->size;
}

bool MemoryAllocator::slabMoreThanGood(MMD* mmd, size_t nBytes)
{
    return neededBlocks(nBytes)<mmd->size;

}

MMD* MemoryAllocator::MaybeCreateAndReturnNextMMD(MMD* wholeSlabMMD, size_t nBytes)
{
    totalAvailableBlocks-=neededBlocks(nBytes);

    if (slabMoreThanGood(wholeSlabMMD, nBytes))
    {
        wholeSlabMMD->size = neededBlocks(nBytes);
        MMD* smallMMD = (MMD*)(
            (char*)wholeSlabMMD+neededBlocks(nBytes)*MEM_BLOCK_SIZE
        );
        *smallMMD = MMD(
        wholeSlabMMD->size - neededBlocks(nBytes),
        wholeSlabMMD->getNext()
        );
        return smallMMD;
    }
    else
    {
        listSize--;
        return wholeSlabMMD->getNext();
    }
}

MMD* MemoryAllocator::createFirstTwoSlabs()
{
    char* bSlabAddr = startAddr + MEM_BLOCK_SIZE;
    char* aSlabAddr = startAddr;
    auto ammd = (MMD*)aSlabAddr - 1;
    auto bmmd = (MMD*)bSlabAddr - 1;
    *ammd = MMD(
        0,
        bmmd
    );
    totalAvailableBlocks = (endAddr - startAddr)/MEM_BLOCK_SIZE - 1;
    *bmmd = MMD(
        totalAvailableBlocks,
        ammd
    );
    return bmmd;
}

char* MemoryAllocator::allocate(size_t nBytes)
{

    if (!initialized) return nullptr;

    //first fit algorithm. also tries merging adjacent slabs.
    auto firstIteratorAddress = iteratorAddress;
    auto cmmd = iteratorAddress->getNext();
    bool secondTry = false;
    while (1)
    {
        if (cmmd==firstIteratorAddress && secondTry) { printInfo(); return nullptr; }
        if (cmmd==firstIteratorAddress) secondTry = true;
        //the second try is because maybe a merge is available. but prob not.


        if (slabGood(cmmd, nBytes))
        {
            auto nextSlabMMDAddr = MaybeCreateAndReturnNextMMD(cmmd, nBytes);
            //now cmmd is out of the list.
            iteratorAddress->setNext(nextSlabMMDAddr);
            cmmd->setAllocated(true);
            return (char*)(cmmd+1);
        }
        else
        {
            //cmmd not needed anymore.
            cmmd=nullptr;
            while (canMerge(iteratorAddress))
            {
                merge(iteratorAddress); //merge with slab above;
            }
        }
        iteratorAddress = iteratorAddress->getNext();
        cmmd = iteratorAddress->getNext();
    }
}

void MemoryAllocator::printInfo()
{
    printString("=== MemoryAllocator Info ===\n");
    printString("startAddr: "); printInteger((uint64)startAddr); printString("\n");
    printString("endAddr:   "); printInteger((uint64)endAddr); printString("\n");
    printString("total blocks: "); printInteger((uint64)((endAddr - startAddr) / MEM_BLOCK_SIZE)); printString("\n");
    printString("free blocks:  "); printInteger(totalAvailableBlocks); printString("\n");
    printString("free bytes:   "); printInteger(totalAvailableBytes()); printString("\n");
    printString("free list size: "); printInteger(listSize); printString("\n");

    printString("--- Free list slabs ---\n");
    MMD* start = iteratorAddress;
    MMD* cur = start;
    uint32 i = 0;
    do
    {
        printString("  ["); printInteger(i); printString("] addr=");
        printInteger((uint64)cur);
        printString(" size="); printInteger(cur->size);
        printString(" blocks ("); printInteger((uint64)cur->size * MEM_BLOCK_SIZE); printString(" bytes)\n");
        cur = cur->getNext();
        i++;
    } while (cur != start);

    printString("===========================\n");
}

int MemoryAllocator::deallocate(char* address)
{
    auto fmmd = (MMD*)address - 1;
    if (!fmmd->isAllocated())
    {
        //TODO: what happens here?
        return -1;
    }
    uint32 nBlocks = fmmd->size;
    totalAvailableBlocks += nBlocks;
    //switchMetadataToFreeSlab(fmmd); //not needed
    fmmd->setNext(iteratorAddress->getNext());
    iteratorAddress->setNext(fmmd);
    listSize++;
    return 0;
}