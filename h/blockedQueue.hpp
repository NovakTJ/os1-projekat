#ifndef _BLOCKED_QUEUE_HPP_
#define _BLOCKED_QUEUE_HPP_

#include "list.hpp"

class TCB;

class BlockedQueue {
private:
    List<TCB> queue;

public:
    TCB *get();
    void put(TCB *tcb);
    bool empty();
    void print();
};

#endif // _BLOCKED_QUEUE_HPP_
