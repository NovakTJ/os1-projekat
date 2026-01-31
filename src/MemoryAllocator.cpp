#include"../h/MemoryAllocator.h"

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

// MMD member function implementations
MMD* MMD::getNext()
{
    return (MMD*)MemoryAllocator::getAdd(nextBID);
}

void MMD::setNext(MMD* a)
{
    nextBID = MemoryAllocator::getBID((char*)a);
}

MMD::MMD(uint32 s, MMD* n)
{
    size = s;
    nextBID = MemoryAllocator::getBID((char*)n);
}

bool MMD::isAllocated()
{
    return nextBID < (uint32)(-1);
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
    return 0; //placeholder, no merging yet. low priority. claude can do it later
}

void MemoryAllocator::merge(MMD* mmd)
{
    //for the disappearing slab its important to find the previous element and update it. O(N) is ok.
    ;
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
        if (cmmd==firstIteratorAddress && secondTry) return nullptr;
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
                break;
                merge(iteratorAddress); //merge with slab above;
            }
        }
        iteratorAddress = iteratorAddress->getNext();
        cmmd = iteratorAddress->getNext();
    }
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