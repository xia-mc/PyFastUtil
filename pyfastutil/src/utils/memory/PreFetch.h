//
// Created by xia__mc on 2024/11/12.
//

#ifndef PYFASTUTIL_PREFETCH_H
#define PYFASTUTIL_PREFETCH_H

#include "Compat.h"

// Detect architecture and compiler
#if defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86)
// x86/x64 platform (Intel/AMD)
#elif defined(__aarch64__) || defined(__arm__)
// ARM platform (32-bit or 64-bit)
    // Using __builtin_prefetch for ARM platforms
    #define _MM_HINT_T0 0  // Prefetch to L1
    #define _MM_HINT_T1 0  // Prefetch to L2 (no direct equivalent, use L1)
    #define _MM_HINT_T2 0  // Prefetch to L3 (no direct equivalent, use L1)
    #define _MM_HINT_NTA 0 // Non-temporal prefetch (no direct equivalent, fallback to L1)

    // Define _mm_prefetch to use __builtin_prefetch for ARM
    #define _mm_prefetch(p, hint) __builtin_prefetch(p, 0, hint)

#else
    // Other platforms: No prefetch available
    #define _mm_prefetch(p, hint) ((void)0)  // No-op for unsupported platforms
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
