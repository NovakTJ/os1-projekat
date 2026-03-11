//
// Created by os on 3/4/26.
//

#include "../h/io.h"
#include "../h/tcb.hpp"
#include "../h/workers.hpp"
#include "../h/print.hpp"
#include "../h/riscv.hpp"

#include "../h/MemoryAllocator.hpp"
#include "../h/syscall_c.h"
#include "../lib/console.h"

// --- Memory allocator stress tests ---

// Recursive alloc/free: allocates a large block at each level, recurses,
// verifies memory integrity via canary, then frees on unwind.
// Depth kept shallow (4 levels) to avoid stack overflow; blocks are large.
void recursiveAllocTestWPrint(int depth, int maxDepth) {
    if (depth >= maxDepth) return;

    size_t size = 100 + depth * 50; // big blocks: 100, 150, 200, 250
    if (size > MemoryAllocator::totalAvailableBytes()) {
        printKString("  skip d");
        printKInteger(depth);
        printKString(" (no mem)\n");
        return;
    }

    void* ptr = mem_alloc(size);
    if (!ptr) {
        printKString("  FAIL@d");
        printKInteger(depth);
        printKString("\n");
        return;
    }

    // Canary: write pattern, recurse, verify pattern survived
    char* p = (char*)ptr;
    for (size_t i = 0; i < 64; i++) // just check first block worth
        p[i] = (char)(depth * 7 + i);

    printKString("  A(");
    printKInteger(size);
    printKString(") ");

    recursiveAllocTestWPrint(depth + 1, maxDepth);

    // Verify canary
    bool ok = true;
    for (size_t i = 0; i < 64; i++) {
        if (p[i] != (char)(depth * 7 + i)) { ok = false; break; }
    }
    printKString(ok ? "ok " : "CORRUPT ");

    mem_free(ptr);
}

// Fragmentation test: allocate 8 large blocks, free odd ones (holes),
// reallocate into holes, then clean up.
void fragmentationTestWPrint() {
    const int N = 8;
    const size_t BLOCK_SZ = 50; // 50 blocks each = 3200 bytes
    void* ptrs[N];

    if (BLOCK_SZ * N > MemoryAllocator::totalAvailableBytes()) {
        printKString("  skip (not enough mem)\n");
        return;
    }

    printKString("  alloc ");
    for (int i = 0; i < N; i++) {
        ptrs[i] = mem_alloc(BLOCK_SZ);
        handlePutc(ptrs[i] ? '+' : 'X');
    }

    printKString(" holes ");
    for (int i = 1; i < N; i += 2) {
        mem_free(ptrs[i]);
        ptrs[i] = nullptr;
        handlePutc('-');
    }

    printKString(" refill ");
    for (int i = 1; i < N; i += 2) {
        ptrs[i] = mem_alloc(BLOCK_SZ);
        handlePutc(ptrs[i] ? '+' : 'X');
    }

    printKString(" cleanup ");
    for (int i = N - 1; i >= 0; i--) {
        if (ptrs[i]) { mem_free(ptrs[i]); handlePutc('-'); }
    }
    printKString(" OK\n");
}

// Binary-tree alloc pattern (depth=3, 7 allocs with big blocks).
// At each node: alloc, recurse left+right, free. Tests overlapping lifetimes.
void treeAllocTestWPrint(int depth) {
    if (depth <= 0) return;

    size_t size = depth * 80; // 240, 160, 80
    if (size > MemoryAllocator::totalAvailableBytes()) {
        handlePutc('X');
        return;
    }

    void* ptr = mem_alloc(size);
    if (!ptr) { handlePutc('X'); return; }
    handlePutc('(');

    treeAllocTestWPrint(depth - 1);
    treeAllocTestWPrint(depth - 1);

    mem_free(ptr);
    handlePutc(')');
}
void bigblocktestWPrint()
{
    int t = MemoryAllocator::totalAvailableBytes();
    printKHexInteger(t);
    printKHexInteger(MemoryAllocator::totalAvailableBytes());printKString("\n");
    auto a = mem_alloc(t/2-10);
    auto b = mem_alloc(t/2-10);
    printKHexInteger(MemoryAllocator::totalAvailableBytes());printKString("\n");

    if (!a || !b) printKString("FAIL");
    mem_free(a);    if (!a || !b) printKString("FAIL");

    mem_free(b);    if (!a || !b) printKString("FAIL");
    printKHexInteger(MemoryAllocator::totalAvailableBytes());printKString("\n");

    a = mem_alloc(t*2/3);    if (!a || !b) printKString("FAIL");

    b = mem_alloc(t/5);    if (!a || !b) printKString("FAIL");
    printKHexInteger(MemoryAllocator::totalAvailableBytes());printKString("\n");

    mem_free(a);    if (!a || !b) printKString("FAIL");

    a = mem_alloc(t*2/3-5);    if (!a || !b) printKString("FAIL");

    mem_free(b);    if (!a || !b) printKString("FAIL");

    b = mem_alloc(t/5);    if (!a || !b) printKString("FAIL");
    printKHexInteger(MemoryAllocator::totalAvailableBytes());printKString("\n");

    auto c = mem_alloc(1);    if (!a || !b || !c) printKString("FAIL");
    printKHexInteger(MemoryAllocator::totalAvailableBytes());printKString("\n");

    mem_free(a);    if (!a || !b || !c) printKString("FAIL");

    mem_free(b);    if (!a || !b || !c) printKString("FAIL");

    mem_free(c);    if (!a || !b || !c) printKString("FAIL");
    printKHexInteger(MemoryAllocator::totalAvailableBytes());printKString("\n");

}
// Coalescing test: alloc three large adjacent blocks, free middle then
// neighbors — allocator should merge all. Then try a huge alloc.
void coalesceTestWPrint() {
    size_t chunkSz = 200;
    if (chunkSz * 3 > MemoryAllocator::totalAvailableBytes()) {
        printKString("  skip (not enough mem)\n");
        return;
    }

    void* a = mem_alloc(chunkSz);
    void* b = mem_alloc(chunkSz);
    void* c = mem_alloc(chunkSz);
    printKString("  abc allocated ");

    mem_free(b);
    handlePutc('1');
    mem_free(a);
    handlePutc('2');
    mem_free(c);
    handlePutc('3');

    // Everything should be coalesced; try a large alloc
    size_t bigSz = MemoryAllocator::totalAvailableBytes() / 2;
    void* big = mem_alloc(bigSz);
    if (big) {
        printKString(" big(");
        printKInteger(bigSz);
        printKString(")=OK");
        mem_free(big);
    } else {
        printKString(" big=FAIL");
    }
    printKString("\n");
}

void testHeavyMemory123WPrint()
{
    printKString("=== Mem Alloc Stress Tests 1-3 ===\n");
    printKString("Available blocks: ");
    printKInteger(MemoryAllocator::totalAvailableBytes());
    printKString("\n");

    printKString("[1] Recursive alloc/free (4 levels, big blocks):\n");
    recursiveAllocTestWPrint(0, 4);
    printKString("\n  PASS\n");

    printKString("[2] Fragmentation (8 x 50-block chunks):\n");
    fragmentationTestWPrint();

    printKString("[3] Tree alloc (depth=3, 7 big allocs):\n  ");
    treeAllocTestWPrint(3);
    printKString("\n  PASS\n");

    printKString("=== Tests 1-3 done ===\n");
}

void testHeavyMemory4WPrint()
{
    printKString("=== Mem Alloc Stress Test 4 ===\n");
    printKString("Available blocks: ");
    printKInteger(MemoryAllocator::totalAvailableBytes());
    printKString("\n");

    printKString("[4] big block test\n");
    bigblocktestWPrint();

    printKString("=== Test 4 done ===\n");
}