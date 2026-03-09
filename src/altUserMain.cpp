#include "../h/syscall_c.h"
#include "../test/printing.hpp"

// =============================================================================
// Multi-phase threading stress test (no getc).
// Phase 1: Producer-consumer (semaphore-based, existing test)
// Phase 2: Barrier synchronization (N threads sync at a barrier point)
// Phase 3: Pipeline (3-stage: generator -> transformer -> accumulator)
// Phase 4: Ping-pong (two threads bouncing a counter back and forth)
// Phase 5: Thread creation storm (many short-lived threads)
// =============================================================================

// ---- Shared helpers ----

static volatile int errorFlag = 0;

static void check(bool cond, const char* msg) {
    if (!cond) {
        printString("FAIL: ");
        printString(msg);
        printString("\n");
        errorFlag = 1;
    }
}

// ============================================================================
// Phase 1: Producer-consumer (kept from original, slightly trimmed)
// ============================================================================

static const int P1_BUF_CAP = 16;
static const int P1_NUM_PRODUCERS = 3;
static const int P1_ITEMS_PER_PRODUCER = 50;

static int p1_buf[P1_BUF_CAP];
static int p1_head = 0, p1_tail = 0;

static sem_t p1_itemAvail, p1_spaceAvail, p1_mutHead, p1_mutTail, p1_waitAll;

static void p1_bufPut(int val) {
    sem_wait(p1_spaceAvail);
    sem_wait(p1_mutTail);
    p1_buf[p1_tail] = val;
    p1_tail = (p1_tail + 1) % P1_BUF_CAP;
    sem_signal(p1_mutTail);
    sem_signal(p1_itemAvail);
}

static int p1_bufGet() {
    sem_wait(p1_itemAvail);
    sem_wait(p1_mutHead);
    int val = p1_buf[p1_head];
    p1_head = (p1_head + 1) % P1_BUF_CAP;
    sem_signal(p1_mutHead);
    sem_signal(p1_spaceAvail);
    return val;
}

static void p1_producer(void* arg) {
    int id = (int)(uint64)arg;
    for (int i = 0; i < P1_ITEMS_PER_PRODUCER; i++) {
        p1_bufPut(id + '0');
        if (i % 10 == 0) thread_dispatch();
    }
    sem_signal(p1_waitAll);
}

static volatile int p1_counts[P1_NUM_PRODUCERS + 1]; // count per producer id

static void p1_consumer(void* arg) {
    (void)arg;
    int total = P1_NUM_PRODUCERS * P1_ITEMS_PER_PRODUCER;
    for (int i = 0; i < total; i++) {
        int ch = p1_bufGet();
        int id = ch - '0';
        if (id >= 1 && id <= P1_NUM_PRODUCERS) p1_counts[id]++;
        putc((char)ch);
        if ((i + 1) % 60 == 0) putc('\n');
        if (i % 20 == 0) thread_dispatch();
    }
    putc('\n');
    sem_signal(p1_waitAll);
}

static void runPhase1() {
    printString("[Phase 1] Producer-consumer: 3 producers x 50 items\n");

    p1_head = 0; p1_tail = 0;
    for (int i = 0; i <= P1_NUM_PRODUCERS; i++) p1_counts[i] = 0;

    sem_open(&p1_itemAvail, 0);
    sem_open(&p1_spaceAvail, P1_BUF_CAP);
    sem_open(&p1_mutHead, 1);
    sem_open(&p1_mutTail, 1);
    sem_open(&p1_waitAll, 0);

    thread_t cons;
    thread_create(&cons, p1_consumer, nullptr);

    thread_t prods[P1_NUM_PRODUCERS];
    for (int i = 0; i < P1_NUM_PRODUCERS; i++)
        thread_create(&prods[i], p1_producer, (void*)(uint64)(i + 1));

    for (int i = 0; i < P1_NUM_PRODUCERS + 1; i++)
        sem_wait(p1_waitAll);

    // Verify each producer contributed exactly ITEMS_PER_PRODUCER items
    for (int i = 1; i <= P1_NUM_PRODUCERS; i++)
        check(p1_counts[i] == P1_ITEMS_PER_PRODUCER, "producer item count mismatch");

    sem_close(p1_itemAvail);
    sem_close(p1_spaceAvail);
    sem_close(p1_mutHead);
    sem_close(p1_mutTail);
    sem_close(p1_waitAll);

    printString("[Phase 1] DONE\n");
}

// ============================================================================
// Phase 2: Barrier synchronization
// N threads each write to their slot, then all wait at a barrier, then each
// thread verifies that ALL slots have been written (proving barrier worked).
// ============================================================================

static const int P2_NUM_THREADS = 6;
static volatile int p2_slots[P2_NUM_THREADS];
static sem_t p2_mutex;
static volatile int p2_arrived;
static sem_t p2_turnstile1;
static sem_t p2_turnstile2;
static sem_t p2_done;

static void p2_barrierPhase1() {
    sem_wait(p2_mutex);
    p2_arrived++;
    if (p2_arrived == P2_NUM_THREADS) {
        // Last thread: open turnstile1 for all
        for (int i = 0; i < P2_NUM_THREADS; i++)
            sem_signal(p2_turnstile1);
    }
    sem_signal(p2_mutex);
    sem_wait(p2_turnstile1);
}

static void p2_barrierPhase2() {
    sem_wait(p2_mutex);
    p2_arrived--;
    if (p2_arrived == 0) {
        // Last thread: open turnstile2 for all
        for (int i = 0; i < P2_NUM_THREADS; i++)
            sem_signal(p2_turnstile2);
    }
    sem_signal(p2_mutex);
    sem_wait(p2_turnstile2);
}

static void p2_worker(void* arg) {
    int id = (int)(uint64)arg;

    // Pre-barrier: write to own slot
    p2_slots[id] = id + 1;
    thread_dispatch();

    // Barrier
    p2_barrierPhase1();

    // Post-barrier: verify ALL slots written
    for (int i = 0; i < P2_NUM_THREADS; i++) {
        check(p2_slots[i] == i + 1, "barrier: slot not set before barrier passed");
    }

    p2_barrierPhase2();

    sem_signal(p2_done);
}

static void runPhase2() {
    printString("[Phase 2] Barrier synchronization: ");
    printInt(P2_NUM_THREADS);
    printString(" threads\n");

    for (int i = 0; i < P2_NUM_THREADS; i++) p2_slots[i] = 0;
    p2_arrived = 0;

    sem_open(&p2_mutex, 1);
    sem_open(&p2_turnstile1, 0);
    sem_open(&p2_turnstile2, 0);
    sem_open(&p2_done, 0);

    thread_t threads[P2_NUM_THREADS];
    for (int i = 0; i < P2_NUM_THREADS; i++)
        thread_create(&threads[i], p2_worker, (void*)(uint64)i);

    for (int i = 0; i < P2_NUM_THREADS; i++)
        sem_wait(p2_done);

    sem_close(p2_mutex);
    sem_close(p2_turnstile1);
    sem_close(p2_turnstile2);
    sem_close(p2_done);

    printString("[Phase 2] DONE\n");
}

// ============================================================================
// Phase 3: Pipeline (3 stages)
// Stage 1 (generator): produces integers 1..N
// Stage 2 (transformer): multiplies each by 3
// Stage 3 (accumulator): sums them up and verifies
// Each stage connected by a semaphore-protected buffer.
// ============================================================================

static const int P3_COUNT = 40;
static const int P3_PIPE_CAP = 8;

// Pipe A: generator -> transformer
static int p3a_buf[P3_PIPE_CAP];
static int p3a_head = 0, p3a_tail = 0;
static sem_t p3a_items, p3a_spaces, p3a_mut;

// Pipe B: transformer -> accumulator
static int p3b_buf[P3_PIPE_CAP];
static int p3b_head = 0, p3b_tail = 0;
static sem_t p3b_items, p3b_spaces, p3b_mut;

static sem_t p3_done;

static void p3_pipePut(int* buf, int cap, int* tail, sem_t spaces, sem_t mut, sem_t items, int val) {
    sem_wait(spaces);
    sem_wait(mut);
    buf[*tail] = val;
    *tail = (*tail + 1) % cap;
    sem_signal(mut);
    sem_signal(items);
}

static int p3_pipeGet(int* buf, int cap, int* head, sem_t items, sem_t mut, sem_t spaces) {
    sem_wait(items);
    sem_wait(mut);
    int val = buf[*head];
    *head = (*head + 1) % cap;
    sem_signal(mut);
    sem_signal(spaces);
    return val;
}

static void p3_generator(void* arg) {
    (void)arg;
    for (int i = 1; i <= P3_COUNT; i++) {
        p3_pipePut(p3a_buf, P3_PIPE_CAP, &p3a_tail, p3a_spaces, p3a_mut, p3a_items, i);
        if (i % 8 == 0) thread_dispatch();
    }
    // Send sentinel
    p3_pipePut(p3a_buf, P3_PIPE_CAP, &p3a_tail, p3a_spaces, p3a_mut, p3a_items, -1);
}

static void p3_transformer(void* arg) {
    (void)arg;
    while (true) {
        int val = p3_pipeGet(p3a_buf, P3_PIPE_CAP, &p3a_head, p3a_items, p3a_mut, p3a_spaces);
        if (val == -1) {
            // Forward sentinel
            p3_pipePut(p3b_buf, P3_PIPE_CAP, &p3b_tail, p3b_spaces, p3b_mut, p3b_items, -1);
            break;
        }
        p3_pipePut(p3b_buf, P3_PIPE_CAP, &p3b_tail, p3b_spaces, p3b_mut, p3b_items, val * 3);
        thread_dispatch();
    }
}

static void p3_accumulator(void* arg) {
    (void)arg;
    int sum = 0;
    while (true) {
        int val = p3_pipeGet(p3b_buf, P3_PIPE_CAP, &p3b_head, p3b_items, p3b_mut, p3b_spaces);
        if (val == -1) break;
        sum += val;
    }
    // Expected: 3 * (1+2+...+N) = 3 * N*(N+1)/2
    int expected = 3 * P3_COUNT * (P3_COUNT + 1) / 2;
    printString("  Pipeline sum = ");
    printInt(sum);
    printString(", expected = ");
    printInt(expected);
    printString("\n");
    check(sum == expected, "pipeline sum mismatch");
    sem_signal(p3_done);
}

static void runPhase3() {
    printString("[Phase 3] Pipeline: generator -> x3 transformer -> accumulator (");
    printInt(P3_COUNT);
    printString(" items)\n");

    p3a_head = 0; p3a_tail = 0;
    p3b_head = 0; p3b_tail = 0;

    sem_open(&p3a_items, 0);
    sem_open(&p3a_spaces, P3_PIPE_CAP);
    sem_open(&p3a_mut, 1);
    sem_open(&p3b_items, 0);
    sem_open(&p3b_spaces, P3_PIPE_CAP);
    sem_open(&p3b_mut, 1);
    sem_open(&p3_done, 0);

    thread_t gen, trans, acc;
    thread_create(&acc, p3_accumulator, nullptr);
    thread_create(&trans, p3_transformer, nullptr);
    thread_create(&gen, p3_generator, nullptr);

    sem_wait(p3_done);

    sem_close(p3a_items); sem_close(p3a_spaces); sem_close(p3a_mut);
    sem_close(p3b_items); sem_close(p3b_spaces); sem_close(p3b_mut);
    sem_close(p3_done);

    printString("[Phase 3] DONE\n");
}

// ============================================================================
// Phase 4: Ping-pong
// Two threads bounce a counter back and forth N times using two semaphores.
// ============================================================================

static const int P4_ROUNDS = 100;
static volatile int p4_counter = 0;
static sem_t p4_ping, p4_pong, p4_finished;

static void p4_playerA(void* arg) {
    (void)arg;
    for (int i = 0; i < P4_ROUNDS; i++) {
        sem_wait(p4_ping);
        p4_counter++;
        sem_signal(p4_pong);
    }
}

static void p4_playerB(void* arg) {
    (void)arg;
    for (int i = 0; i < P4_ROUNDS; i++) {
        sem_wait(p4_pong);
        p4_counter++;
        sem_signal(p4_ping);
    }
    sem_signal(p4_finished);
}

static void runPhase4() {
    printString("[Phase 4] Ping-pong: ");
    printInt(P4_ROUNDS);
    printString(" rounds\n");

    p4_counter = 0;
    sem_open(&p4_ping, 1);  // A goes first
    sem_open(&p4_pong, 0);
    sem_open(&p4_finished, 0);

    thread_t a, b;
    thread_create(&a, p4_playerA, nullptr);
    thread_create(&b, p4_playerB, nullptr);

    sem_wait(p4_finished);

    int expected = P4_ROUNDS * 2;
    printString("  Counter = ");
    printInt(p4_counter);
    printString(", expected = ");
    printInt(expected);
    printString("\n");
    check(p4_counter == expected, "ping-pong counter mismatch");

    sem_close(p4_ping);
    sem_close(p4_pong);
    sem_close(p4_finished);

    printString("[Phase 4] DONE\n");
}

// ============================================================================
// Phase 5: Thread creation storm
// Rapidly create many short-lived threads. Each thread increments a shared
// counter (protected by a semaphore) and exits. Tests thread lifecycle.
// ============================================================================

static const int P5_NUM_THREADS = 30;
static volatile int p5_counter = 0;
static sem_t p5_mutex, p5_allDone;

static void p5_shortTask(void* arg) {
    int id = (int)(uint64)arg;
    (void)id;

    // Small amount of work
    for (volatile int i = 0; i < 50; i++) {}

    sem_wait(p5_mutex);
    p5_counter++;
    int c = p5_counter;
    sem_signal(p5_mutex);

    if (c == P5_NUM_THREADS) {
        sem_signal(p5_allDone);
    }
}

static void runPhase5() {
    printString("[Phase 5] Thread storm: ");
    printInt(P5_NUM_THREADS);
    printString(" short-lived threads\n");

    p5_counter = 0;
    sem_open(&p5_mutex, 1);
    sem_open(&p5_allDone, 0);

    thread_t threads[P5_NUM_THREADS];
    for (int i = 0; i < P5_NUM_THREADS; i++) {
        thread_create(&threads[i], p5_shortTask, (void*)(uint64)i);
        // Occasionally yield to let threads run and finish
        if (i % 5 == 4) thread_dispatch();
    }

    sem_wait(p5_allDone);

    printString("  Threads completed: ");
    printInt(p5_counter);
    printString("/");
    printInt(P5_NUM_THREADS);
    printString("\n");
    check(p5_counter == P5_NUM_THREADS, "thread storm: not all threads completed");

    sem_close(p5_mutex);
    sem_close(p5_allDone);

    printString("[Phase 5] DONE\n");
}

// ============================================================================
// Entry point
// ============================================================================

void altUserMain() {
    printString("========================================\n");
    printString("  Multi-phase threading stress test\n");
    printString("========================================\n\n");

    runPhase1();
    printString("\n");

    runPhase2();
    printString("\n");

    runPhase3();
    printString("\n");

    runPhase4();
    printString("\n");

    runPhase5();

    printString("\n========================================\n");
    if (errorFlag) {
        printString("  SOME TESTS FAILED\n");
    } else {
        printString("  ALL TESTS PASSED\n");
    }
    printString("========================================\n");
}
