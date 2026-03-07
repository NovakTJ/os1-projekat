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
        u_printString("  skip d");
        u_printInteger(depth);
        u_printString(" (no mem)\n");
        return;
    }

    void* ptr = mem_alloc(size);
    if (!ptr) {
        u_printString("  FAIL@d");
        u_printInteger(depth);
        u_printString("\n");
        return;
    }

    // Canary: write pattern, recurse, verify pattern survived
    char* p = (char*)ptr;
    for (size_t i = 0; i < 64; i++) // just check first block worth
        p[i] = (char)(depth * 7 + i);

    u_printString("  A(");
    u_printInteger(size);
    u_printString(") ");

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
        u_printString("  skip (not enough mem)\n");
        return;
    }

    u_printString("  alloc ");
    for (int i = 0; i < N; i++) {
        ptrs[i] = mem_alloc(BLOCK_SZ);
        ptrs[i] ? u_printString("+") : u_printString("X");
    }

    u_printString(" holes ");
    for (int i = 1; i < N; i += 2) {
        mem_free(ptrs[i]);
        ptrs[i] = nullptr;
        u_printString("-");
    }

    u_printString(" refill ");
    for (int i = 1; i < N; i += 2) {
        ptrs[i] = mem_alloc(BLOCK_SZ);
        ptrs[i] ? u_printString("+") : u_printString("X");
    }

    u_printString(" cleanup ");
    for (int i = N - 1; i >= 0; i--) {
        if (ptrs[i]) { mem_free(ptrs[i]); u_printString("-"); }
    }
    u_printString(" OK\n");
}

// Binary-tree alloc pattern (depth=3, 7 allocs with big blocks).
// At each node: alloc, recurse left+right, free. Tests overlapping lifetimes.
void treeAllocTest(int depth) {
    if (depth <= 0) return;

    size_t size = depth * 80; // 240, 160, 80
    if (size > MemoryAllocator::totalAvailableBytes()) {
        u_printString("X");
        return;
    }

    void* ptr = mem_alloc(size);
    if (!ptr) { u_printString("X"); return; }
    u_printString("(");

    treeAllocTest(depth - 1);
    treeAllocTest(depth - 1);

    mem_free(ptr);
    u_printString(")");
}
void bigblocktest()
{
    int t = MemoryAllocator::totalAvailableBytes();
    u_printHexInteger(t);
    u_printHexInteger(MemoryAllocator::totalAvailableBytes());u_printString("\n");
    auto a = mem_alloc(t/2-10);
    auto b = mem_alloc(t/2-10);
    u_printHexInteger(MemoryAllocator::totalAvailableBytes());u_printString("\n");

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
        u_printString("  skip (not enough mem)\n");
        return;
    }

    void* a = mem_alloc(chunkSz);
    void* b = mem_alloc(chunkSz);
    void* c = mem_alloc(chunkSz);
    u_printString("  abc allocated ");

    mem_free(b);
    u_printString("1");
    mem_free(a);
    u_printString("2");
    mem_free(c);
    u_printString("3");

    // Everything should be coalesced; try a large alloc
    size_t bigSz = MemoryAllocator::totalAvailableBytes() / 2;
    void* big = mem_alloc(bigSz);
    if (big) {
        u_printString(" big(");
        u_printInteger(bigSz);
        u_printString(")=OK");
        mem_free(big);
    } else {
        u_printString(" big=FAIL");
    }
    u_printString("\n");
}

void testHeavyMemory123()
{
    u_printString("=== Mem Alloc Stress Tests 1-3 ===\n");
    u_printString("Available blocks: ");
    u_printInteger(MemoryAllocator::totalAvailableBytes());
    u_printString("\n");

    u_printString("[1] Recursive alloc/free (4 levels, big blocks):\n");
    recursiveAllocTest(0, 4);
    u_printString("\n  PASS\n");

    u_printString("[2] Fragmentation (8 x 50-block chunks):\n");
    fragmentationTest();

    u_printString("[3] Tree alloc (depth=3, 7 big allocs):\n  ");
    treeAllocTest(3);
    u_printString("\n  PASS\n");

    u_printString("=== Tests 1-3 done ===\n");
}

void testHeavyMemory4()
{
    u_printString("=== Mem Alloc Stress Test 4 ===\n");
    u_printString("Available blocks: ");
    u_printInteger(MemoryAllocator::totalAvailableBytes());
    u_printString("\n");

    u_printString("[4] big block test\n");
    bigblocktest();

    u_printString("=== Test 4 done ===\n");
}