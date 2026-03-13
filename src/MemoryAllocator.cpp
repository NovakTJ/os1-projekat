#include"../h/MemoryAllocator.hpp"
#include"../h/print.hpp"
#include"../h/riscv.hpp"

// Known bugs:
// - totalAvailableBlocks may underflow under extreme stress (unsigned wrap)
// - Failed allocation can cause a kernel trap instead of returning nullptr;
//   usually because callers don't null-check the return value, not this code

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

size_t MemoryAllocator::largestAvailableBlock()
{
    if (!initialized) return 0;
    uint32 maxBlocks = 0;
    MMD* start = iteratorAddress;
    MMD* cur = start;
    do
    {
        uint32 total = cur->size;
        MMD* tmp = cur;
        while (canMerge(tmp))
        {
            tmp = (MMD*)((char*)tmp + tmp->size * MEM_BLOCK_SIZE);
            total += tmp->size;
        }
        if (total > maxBlocks)
            maxBlocks = total;
        cur = cur->getNext();
    } while (cur != start);
    if (maxBlocks == 0) return 0;
    return (size_t)(maxBlocks - 1) * MEM_BLOCK_SIZE + (MEM_BLOCK_SIZE - MMDSIZE);
}

size_t MemoryAllocator::MMDSIZE = 8;




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
    MMD* aboveMmd = (MMD*)((char*)mmd + mmd->size * MEM_BLOCK_SIZE);
    if ((char*)(aboveMmd + 1) >= endAddr) return false;
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
size_t MemoryAllocator::neededBytes(size_t nBlocks)
{
    return (nBlocks-1)*MEM_BLOCK_SIZE + 1;
    //
}
uint32 MemoryAllocator::neededBlocks(size_t nBytes)
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
    /*plan 1:
     *just calculate the n of bytes before calling memoryallocator. this is shitty code but i wont rework it.
     *plan 2:
     *rework allocate to work w blocks and not bytes. too difficult.
     */
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
        MMD* smallMMD = (MMD*)(
            (char*)wholeSlabMMD+neededBlocks(nBytes)*MEM_BLOCK_SIZE
        );
        *smallMMD = MMD(
        wholeSlabMMD->size - neededBlocks(nBytes),
        wholeSlabMMD->getNext()
        );
        wholeSlabMMD->size = neededBlocks(nBytes);
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

char* MemoryAllocator::allocateBytes(size_t nBytes)
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

char* MemoryAllocator::allocateBlocks(size_t nBlocks)
{
    return allocateBytes(neededBytes(nBlocks));
}

void MemoryAllocator::printInfo()
{
    printKString("=== MemoryAllocator Info ===\n");
    printKString("startAddr: "); printKHexInteger((uint64)startAddr); printKString("\n");
    printKString("endAddr:   "); printKHexInteger((uint64)endAddr); printKString("\n");
    printKString("total blocks: "); printKInteger((uint64)((endAddr - startAddr) / MEM_BLOCK_SIZE)); printKString("\n");
    printKString("free blocks:  "); printKInteger(totalAvailableBlocks); printKString("\n");
    printKString("free bytes:   "); printKHexInteger(totalAvailableBytes()); printKString("\n");
    printKString("free list size: "); printKInteger(listSize); printKString("\n");

    printKString("--- Free list slabs ---\n");
    MMD* start = iteratorAddress;
    MMD* cur = start;
    uint32 i = 0;
    do
    {
        printKString("  ["); printKInteger(i); printKString("] addr=");
        printKHexInteger((uint64)cur);
        printKString(" size="); printKInteger(cur->size);
        printKString(" blocks ("); printKHexInteger((uint64)cur->size * MEM_BLOCK_SIZE); printKString(" bytes)\n");
        cur = cur->getNext();
        i++;
    } while (cur != start);

    printKString("===========================\n");
}

void MemoryAllocator::u_printInfo()
{
    printUString("=== MemoryAllocator Info ===\n");
    printUString("startAddr: "); printUHexInteger((uint64)startAddr); printUString("\n");
    printUString("endAddr:   "); printUHexInteger((uint64)endAddr); printUString("\n");
    printUString("total blocks: "); printUInteger((uint64)((endAddr - startAddr) / MEM_BLOCK_SIZE)); printUString("\n");
    printUString("free blocks:  "); printUInteger(totalAvailableBlocks); printUString("\n");
    printUString("free bytes:   "); printUHexInteger(totalAvailableBytes()); printUString("\n");
    printUString("free list size: "); printUInteger(listSize); printUString("\n");

    printUString("--- Free list slabs ---\n");
    MMD* start = iteratorAddress;
    MMD* cur = start;
    uint32 i = 0;
    do
    {
        printUString("  ["); printUInteger(i); printUString("] addr=");
        printUHexInteger((uint64)cur);
        printUString(" size="); printUInteger(cur->size);
        printUString(" blocks ("); printUHexInteger((uint64)cur->size * MEM_BLOCK_SIZE); printUString(" bytes)\n");
        cur = cur->getNext();
        i++;
    } while (cur != start);

    printUString("===========================\n");
}

int MemoryAllocator::deallocate(char* address)
{
    auto fmmd = (MMD*)address - 1;
    if (!fmmd->isAllocated())
    {
        printKString("double free\n");
        printKHexInteger((uint64)fmmd);
        printKString("\n");
        printInfo();
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