//
// Created by os on 3/7/26.
//
#include "../h/tcb.hpp"
#include "../h/_buffer.hpp"

void putIntoInputBuffer(char c)
{
    _buf::ib->putIfNotFull(c);
}

char handleGetc()
{
    return _buf::ib->get(); //blocking
}

void handlePutc(char c)
{
    _buf::ob->putIfNotFull(c);
}
extern volatile bool oThreadStop;

void TCB::OThreadBody(void* arg)
{
    (void)arg;
    while (1)
    {
        if (oThreadStop && !_buf::ob->getCnt()) break;

        while (!_buf::ob->getCnt()) {
            if (oThreadStop) break;
            kDispatch();
        }
        if (oThreadStop && !_buf::ob->getCnt()) break;

        while (!(*((volatile uint8*)CONSOLE_STATUS) & CONSOLE_TX_STATUS_BIT));

        char tosend = _buf::ob->get();
        while (!(*((volatile uint8*)CONSOLE_STATUS) & CONSOLE_TX_STATUS_BIT));
        *((volatile char*)CONSOLE_TX_DATA) = tosend;
    }
}