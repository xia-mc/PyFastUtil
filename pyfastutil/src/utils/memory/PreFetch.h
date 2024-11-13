//
// Created by xia__mc on 2024/11/12.
//

#ifndef PYFASTUTIL_PREFETCH_H
#define PYFASTUTIL_PREFETCH_H

#include "Compat.h"

#ifdef __clang__
#define _MM_HINT_T0 3 // NOLINT(*-reserved-identifier)
#define _MM_HINT_T1 2 // NOLINT(*-reserved-identifier)
#define _MM_HINT_T2 1 // NOLINT(*-reserved-identifier)
#define _MM_HINT_NTA 0 // NOLINT(*-reserved-identifier)
#endif

__forceinline void prefetchL1(const void *pointer) {
    _mm_prefetch(reinterpret_cast<const char *>(pointer), _MM_HINT_T0);
}

__forceinline void prefetchL2(const void *pointer) {
    _mm_prefetch(reinterpret_cast<const char *>(pointer), _MM_HINT_T1);
}

__forceinline void prefetchL3(const void *pointer) {
    _mm_prefetch(reinterpret_cast<const char *>(pointer), _MM_HINT_T2);
}

__forceinline void prefetchNTA(const void *pointer) {
    _mm_prefetch(reinterpret_cast<const char *>(pointer), _MM_HINT_NTA);
}

#endif //PYFASTUTIL_PREFETCH_H
