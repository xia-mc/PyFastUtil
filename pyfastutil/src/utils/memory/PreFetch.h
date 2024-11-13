//
// Created by xia__mc on 2024/11/12.
//

#ifndef PYFASTUTIL_PREFETCH_H
#define PYFASTUTIL_PREFETCH_H

#include <cstdlib>
#include "Compat.h"

__forceinline void prefetchL1(const void *pointer) {
    _mm_prefetch(reinterpret_cast<const char *>(pointer), 3);  // _MM_HINT_T0
}

__forceinline void prefetchL2(const void *pointer) {
    _mm_prefetch(reinterpret_cast<const char *>(pointer), 2);  // _MM_HINT_T1
}

__forceinline void prefetchL3(const void *pointer) {
    _mm_prefetch(reinterpret_cast<const char *>(pointer), 1);  // _MM_HINT_T2
}

__forceinline void prefetchNTA(const void *pointer) {
    _mm_prefetch(reinterpret_cast<const char *>(pointer), 0);  // _MM_HINT_T3
}

#endif //PYFASTUTIL_PREFETCH_H
