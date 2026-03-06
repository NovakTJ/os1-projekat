//
// Created by os on 3/4/26.
//

#include "../h/tcb.hpp"
#include "../h/workers.hpp"
#include "../h/print.hpp"
#include "../h/riscv.hpp"

#include "../h/MemoryAllocator.h"
#include "../lib/console.h"
#include "../lib/mem.h"

// --- Memory allocator stress tests ---

// Recursive alloc/free: allocates a large block at each level, recurses,
// verifies memory integrity via canary, then frees on unwind.
// Depth kept shallow (4 levels) to avoid stack overflow; blocks are large.
void recursiveAllocTest(int depth, int maxDepth) {
    if (depth >= maxDepth) return;

    size_t size = 100 + depth * 50; // big blocks: 100, 150, 200, 250
    if (size > MemoryAllocator::totalAvailableBytes()) {
        printString("  skip d");
        printInteger(depth);
        printString(" (no mem)\n");
        return;
    }

    void* ptr = mem_alloc(size);
    if (!ptr) {
        printString("  FAIL@d");
        printInteger(depth);
        printString("\n");
        return;
    }

    // Canary: write pattern, recurse, verify pattern survived
    char* p = (char*)ptr;
    for (size_t i = 0; i < 64; i++) // just check first block worth
        p[i] = (char)(depth * 7 + i);

    printString("  A(");
    printInteger(size);
    printString(") ");

    recursiveAllocTest(depth + 1, maxDepth);

    // Verify canary
    bool ok = true;
    for (size_t i = 0; i < 64; i++) {
        if (p[i] != (char)(depth * 7 + i)) { ok = false; break; }
    }
    printString(ok ? "ok " : "CORRUPT ");

    mem_free(ptr);
}

// Fragmentation test: allocate 8 large blocks, free odd ones (holes),
// reallocate into holes, then clean up.
void fragmentationTest() {
    const int N = 8;
    const size_t BLOCK_SZ = 50; // 50 blocks each = 3200 bytes
    void* ptrs[N];

    if (BLOCK_SZ * N > MemoryAllocator::totalAvailableBytes()) {
        printString("  skip (not enough mem)\n");
        return;
    }

    printString("  alloc ");
    for (int i = 0; i < N; i++) {
        ptrs[i] = mem_alloc(BLOCK_SZ);
        __putc(ptrs[i] ? '+' : 'X');
    }

    printString(" holes ");
    for (int i = 1; i < N; i += 2) {
        mem_free(ptrs[i]);
        ptrs[i] = nullptr;
        __putc('-');
    }

    printString(" refill ");
    for (int i = 1; i < N; i += 2) {
        ptrs[i] = mem_alloc(BLOCK_SZ);
        __putc(ptrs[i] ? '+' : 'X');
    }

    printString(" cleanup ");
    for (int i = N - 1; i >= 0; i--) {
        if (ptrs[i]) { mem_free(ptrs[i]); __putc('-'); }
    }
    printString(" OK\n");
}

// Binary-tree alloc pattern (depth=3, 7 allocs with big blocks).
// At each node: alloc, recurse left+right, free. Tests overlapping lifetimes.
void treeAllocTest(int depth) {
    if (depth <= 0) return;

    size_t size = depth * 80; // 240, 160, 80
    if (size > MemoryAllocator::totalAvailableBytes()) {
        __putc('X');
        return;
    }

    void* ptr = mem_alloc(size);
    if (!ptr) { __putc('X'); return; }
    __putc('(');

    treeAllocTest(depth - 1);
    treeAllocTest(depth - 1);

    mem_free(ptr);
    __putc(')');
}
void bigblocktest()
{
    int t = MemoryAllocator::totalAvailableBytes();
    printHexInteger(t);
    printHexInteger(MemoryAllocator::totalAvailableBytes());printString("\n");
    auto a = mem_alloc(t/2-10);
    auto b = mem_alloc(t/2-10);
    printHexInteger(MemoryAllocator::totalAvailableBytes());printString("\n");

    if (!a || !b) printString("FAIL");
    mem_free(a);    if (!a || !b) printString("FAIL");

    mem_free(b);    if (!a || !b) printString("FAIL");
    printHexInteger(MemoryAllocator::totalAvailableBytes());printString("\n");

    a = mem_alloc(t*2/3);    if (!a || !b) printString("FAIL");

    b = mem_alloc(t/5);    if (!a || !b) printString("FAIL");
    printHexInteger(MemoryAllocator::totalAvailableBytes());printString("\n");

    mem_free(a);    if (!a || !b) printString("FAIL");

    a = mem_alloc(t*2/3-5);    if (!a || !b) printString("FAIL");

    mem_free(b);    if (!a || !b) printString("FAIL");

    b = mem_alloc(t/5);    if (!a || !b) printString("FAIL");
    printHexInteger(MemoryAllocator::totalAvailableBytes());printString("\n");

    auto c = mem_alloc(1);    if (!a || !b || !c) printString("FAIL");
    printHexInteger(MemoryAllocator::totalAvailableBytes());printString("\n");

    mem_free(a);    if (!a || !b || !c) printString("FAIL");

    mem_free(b);    if (!a || !b || !c) printString("FAIL");

    mem_free(c);    if (!a || !b || !c) printString("FAIL");
    printHexInteger(MemoryAllocator::totalAvailableBytes());printString("\n");

}
// Coalescing test: alloc three large adjacent blocks, free middle then
// neighbors — allocator should merge all. Then try a huge alloc.
void coalesceTest() {
    size_t chunkSz = 200;
    if (chunkSz * 3 > MemoryAllocator::totalAvailableBytes()) {
        printString("  skip (not enough mem)\n");
        return;
    }

    void* a = mem_alloc(chunkSz);
    void* b = mem_alloc(chunkSz);
    void* c = mem_alloc(chunkSz);
    printString("  abc allocated ");

    mem_free(b);
    __putc('1');
    mem_free(a);
    __putc('2');
    mem_free(c);
    __putc('3');

    // Everything should be coalesced; try a large alloc
    size_t bigSz = MemoryAllocator::totalAvailableBytes() / 2;
    void* big = mem_alloc(bigSz);
    if (big) {
        printString(" big(");
        printInteger(bigSz);
        printString(")=OK");
        mem_free(big);
    } else {
        printString(" big=FAIL");
    }
    printString("\n");
}

void testHeavyMemory()
{
    printString("=== Mem Alloc Stress Tests ===\n");
    printString("Available blocks: ");
    printInteger(MemoryAllocator::totalAvailableBytes());
    printString("\n");
    /*
        printString("[1] Recursive alloc/free (4 levels, big blocks):\n");
        recursiveAllocTest(0, 4);
        printString("\n  PASS\n");

        printString("[2] Fragmentation (8 x 50-block chunks):\n");
        fragmentationTest();

        printString("[3] Tree alloc (depth=3, 7 big allocs):\n  ");
        treeAllocTest(3);
        printString("\n  PASS\n");

        printString("[4] Coalescing (3 x 200-block chunks):\n");
        coalesceTest();
        */
    printString("[4] big block test\n");
    bigblocktest();
    printString("=== All tests done ===\n");
}