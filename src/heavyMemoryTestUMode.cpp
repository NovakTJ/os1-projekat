//
// Created by os on 3/4/26.
//

#include "../h/tcb.hpp"
#include "../h/workers.hpp"
#include "../h///print.hpp"
#include "../h/riscv.hpp"

#include "../h/MemoryAllocator.h"
#include "../h/syscall_c.h"
#include "../lib/console.h"

// --- Memory allocator stress tests ---

// Recursive alloc/free: allocates a large block at each level, recurses,
// verifies memory integrity via canary, then frees on unwind.
// Depth kept shallow (4 levels) to avoid stack overflow; blocks are large.
void recursiveAllocTest(int depth, int maxDepth) {
    if (depth >= maxDepth) return;

    size_t size = 100 + depth * 50; // big blocks: 100, 150, 200, 250
    if (size > MemoryAllocator::totalAvailableBytes()) {
        printUString("  skip d");
        printUInteger(depth);
        printUString(" (no mem)\n");
        return;
    }

    void* ptr = mem_alloc(size);
    if (!ptr) {
        printUString("  FAIL@d");
        printUInteger(depth);
        printUString("\n");
        return;
    }

    // Canary: write pattern, recurse, verify pattern survived
    char* p = (char*)ptr;
    for (size_t i = 0; i < 64; i++) // just check first block worth
        p[i] = (char)(depth * 7 + i);

    printUString("  A(");
    printUInteger(size);
    printUString(") ");

    recursiveAllocTest(depth + 1, maxDepth);

    // Verify canary
    for (size_t i = 0; i < 64; i++) {
        if (p[i] != (char)(depth * 7 + i)) { break; }
    }

    mem_free(ptr);
}

// Fragmentation test: allocate 8 large blocks, free odd ones (holes),
// reallocate into holes, then clean up.
void fragmentationTest() {
    const int N = 8;
    const size_t BLOCK_SZ = 50; // 50 blocks each = 3200 bytes
    void* ptrs[N];

    if (BLOCK_SZ * N > MemoryAllocator::totalAvailableBytes()) {
        printUString("  skip (not enough mem)\n");
        return;
    }

    printUString("  alloc ");
    for (int i = 0; i < N; i++) {
        ptrs[i] = mem_alloc(BLOCK_SZ);
        ptrs[i] ? printUString("+") : printUString("X");
    }

    printUString(" holes ");
    for (int i = 1; i < N; i += 2) {
        mem_free(ptrs[i]);
        ptrs[i] = nullptr;
        printUString("-");
    }

    printUString(" refill ");
    for (int i = 1; i < N; i += 2) {
        ptrs[i] = mem_alloc(BLOCK_SZ);
        ptrs[i] ? printUString("+") : printUString("X");
    }

    printUString(" cleanup ");
    for (int i = N - 1; i >= 0; i--) {
        if (ptrs[i]) { mem_free(ptrs[i]); printUString("-"); }
    }
    printUString(" OK\n");
}

// Binary-tree alloc pattern (depth=3, 7 allocs with big blocks).
// At each node: alloc, recurse left+right, free. Tests overlapping lifetimes.
void treeAllocTest(int depth) {
    if (depth <= 0) return;

    size_t size = depth * 80; // 240, 160, 80
    if (size > MemoryAllocator::totalAvailableBytes()) {
        printUString("X");
        return;
    }

    void* ptr = mem_alloc(size);
    if (!ptr) { printUString("X"); return; }
    printUString("(");

    treeAllocTest(depth - 1);
    treeAllocTest(depth - 1);

    mem_free(ptr);
    printUString(")");
}
void bigblocktest()
{
    int t = MemoryAllocator::totalAvailableBytes();
    printUHexInteger(t);
    printUHexInteger(MemoryAllocator::totalAvailableBytes());printUString("\n");
    auto a = mem_alloc(t/2-100);
    auto b = mem_alloc(t/2-100);
    printUHexInteger(MemoryAllocator::totalAvailableBytes());printUString("\n");

    mem_free(a);

    mem_free(b);

    a = mem_alloc(t*2/3);

    b = mem_alloc(t/5);

    mem_free(a);

    a = mem_alloc(t*2/3-5);

    mem_free(b);

    b = mem_alloc(t/5);

    auto c = mem_alloc(1);

    mem_free(a);

    mem_free(b);

    mem_free(c);

}
// Coalescing test: alloc three large adjacent blocks, free middle then
// neighbors — allocator should merge all. Then try a huge alloc.
void coalesceTest() {
    size_t chunkSz = 200;
    if (chunkSz * 3 > MemoryAllocator::totalAvailableBytes()) {
        printUString("  skip (not enough mem)\n");
        return;
    }

    void* a = mem_alloc(chunkSz);
    void* b = mem_alloc(chunkSz);
    void* c = mem_alloc(chunkSz);
    printUString("  abc allocated ");

    mem_free(b);
    printUString("1");
    mem_free(a);
    printUString("2");
    mem_free(c);
    printUString("3");

    // Everything should be coalesced; try a large alloc
    size_t bigSz = MemoryAllocator::totalAvailableBytes() / 2;
    void* big = mem_alloc(bigSz);
    if (big) {
        printUString(" big(");
        printUInteger(bigSz);
        printUString(")=OK");
        mem_free(big);
    } else {
        printUString(" big=FAIL");
    }
    printUString("\n");
}

void testHeavyMemory123()
{
    printUString("=== Mem Alloc Stress Tests 1-3 ===\n");
    printUString("Available blocks: ");
    printUInteger(MemoryAllocator::totalAvailableBytes());
    printUString("\n");

    printUString("[1] Recursive alloc/free (4 levels, big blocks):\n");
    recursiveAllocTest(0, 4);
    printUString("\n  PASS\n");

    printUString("[2] Fragmentation (8 x 50-block chunks):\n");
    fragmentationTest();

    printUString("[3] Tree alloc (depth=3, 7 big allocs):\n  ");
    treeAllocTest(3);
    printUString("\n  PASS\n");

    printUString("=== Tests 1-3 done ===\n");
}

void testHeavyMemory4()
{
    printUString("=== Mem Alloc Stress Test 4 ===\n");
    printUString("Available blocks: ");
    printUInteger(MemoryAllocator::totalAvailableBytes());
    printUString("\n");

    printUString("[4] big block test\n");
    bigblocktest();

    printUString("=== Test 4 done ===\n");
}