//
// Test 1: MemoryAllocator initialization
//
#include "../h/MemoryAllocator.h"
#include "../lib/console.h"

void main()
{
    __putc('I');
    __putc('n');
    __putc('i');
    __putc('t');
    __putc('\n');

    MemoryAllocator::init();    __putc('D');
    __putc('o');
    __putc('n');
    __putc('e');
    __putc('\n');

    char* ptr = MemoryAllocator::allocate(3);
    MemoryAllocator::deallocate(ptr);
    char* ptr2 = MemoryAllocator::allocate(3);
    ptr = MemoryAllocator::allocate(3);
    MemoryAllocator::deallocate(ptr);
    ptr = MemoryAllocator::allocate(3);
    MemoryAllocator::deallocate(ptr);
    MemoryAllocator::deallocate(ptr2);
__putc('!');
    while(1);  // Halt
}
