//
// Created by os on 3/9/26.
//

#ifndef OS1_PROJEKAT_BUFFER_H
#define OS1_PROJEKAT_BUFFER_H

#include"../h/semaphore.hpp"
#include"../h/MemoryAllocator.hpp"
typedef int bi_t;
class _buf {
private:
    bi_t _cap;
    char *buffer;
    bi_t head, tail;

    _sem* spaceAvailable;
    _sem* itemAvailable;
    _sem* mutexHead;
    _sem* mutexTail;

public:
    void* operator new(size_t size) { return MemoryAllocator::allocateBytes(size); }
    void operator delete(void* ptr) { MemoryAllocator::deallocate((char*)ptr); }
    void* operator new[](size_t size) { return MemoryAllocator::allocateBytes(size); }
    void operator delete[](void* ptr) { MemoryAllocator::deallocate((char*)ptr); }

    static _buf* ob;
    static _buf* ib;
    static void initBuffers();

    _buf(bi_t _cap);
    virtual ~_buf();
    void virtual putBlocking(char val);
    void virtual putIfNotFull(char val); //nonblocking!
    char virtual get();
    bi_t getCnt();

};
// class _obuf : public _buf
// {
//     void put(char val) override; //needs to be non blocking. if full then return.
//     char get() override;
// };
// class _ibuf : public _buf
// {
//     void put(char val) override;
//     char get() override;
// };

#endif //OS1_PROJEKAT_BUFFER_H