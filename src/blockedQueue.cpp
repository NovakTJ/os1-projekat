#include "../h/blockedQueue.hpp"
#include "../h/tcb.hpp"
#include "../h/print.hpp"

TCB *BlockedQueue::get() {
    return queue.removeFirst();
}

void BlockedQueue::put(TCB *tcb) {
    tcb->setCurrentQueue(3);
    queue.addLast(tcb);
}

bool BlockedQueue::empty() {
    return queue.empty();
}

void BlockedQueue::print() {
    printKString("--- Blocked Queue ---\n");
    queue.printListOfTCBs();
    printKString("---------------------\n");
}
