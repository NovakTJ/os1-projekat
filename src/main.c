//
// Created by os on 1/30/26.
//
#include"../lib/console.h"
void main()
{
    char x;
    while (1)
    {
        x = __getc();
        __putc(x);
    }
}