#pragma once

#include "hw.h"

/*
 * The functions are intended to be used in supervisor mode, and they are not thread-safe
 */


#ifdef __cplusplus
    extern "C" {
#endif


    void* __mem_alloc(size_t size);

    int __mem_free(void* ptr);
        size_t __mem_get_free_space();
        size_t __mem_get_largest_free_block();

#ifdef __cplusplus
    }
#endif


