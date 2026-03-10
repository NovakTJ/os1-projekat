//
// Created by os on 3/9/26.
//

#include "../h/_buffer.hpp"

_buf* _buf::ob = nullptr;
_buf* _buf::ib = nullptr;

void _buf::initBuffers() {
    ob = new _buf(256);
    ib = new _buf(256);
}

_buf::_buf(bi_t _cap) : _cap(_cap)
{
    buffer = (char*)MemoryAllocator::allocateBytes(_cap);
    mutexHead = new _sem(1);
    mutexTail = nullptr; //unused
    //spaceAvailable = new _sem(_cap-1); // only needed if putBlocking is used
    spaceAvailable = nullptr;
    itemAvailable = new _sem(0);
    head = 0;
    tail = 0;

}
_buf::~_buf()
{
    delete mutexHead;
    //delete mutexTail;
    //delete spaceAvailable; // only needed if putBlocking is used
    delete itemAvailable;
    MemoryAllocator::deallocate(buffer);
}

// NOTE: if you ever re-enable putBlocking, you must also re-enable spaceAvailable
// (uncomment in constructor, destructor, and get()) since putBlocking relies on it.
//void _buf::putBlocking(char val)
//{
//    spaceAvailable->wait();
//    mutexTail->wait();
//    buffer[tail] = val;
//    tail = (tail + 1) % _cap;
//    mutexTail->signal();
//    itemAvailable->signal();
//}
void _buf::putIfNotFull(char val)
{
    if ((tail+1)%_cap == head) { return; }
    buffer[tail] = val;
    tail = (tail + 1) % _cap;
    itemAvailable->signal();


}
char _buf::get()
{
    itemAvailable->wait();
    mutexHead->wait();
    char ret = buffer[head];
    head = (head + 1) % _cap;
    mutexHead->signal();
    //spaceAvailable->signal(); // only needed if putBlocking is used
    return ret;
}
bi_t _buf::getCnt()
{
    return ((tail+_cap)-head) % _cap;
}