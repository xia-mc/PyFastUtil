//
// Created by xia__mc on 2024/11/12.
//

#ifndef PYFASTUTIL_SIMD_UTILS_H
#define PYFASTUTIL_SIMD_UTILS_H

#if !defined(_MSC_VER)

#include "cstring"

#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)

#include <immintrin.h>

#endif

#include "SIMDHelper.h"
#include "algorithm"
#include "utils/memory/PreFetch.h"
#include "utils/include/qreverse.hpp"

namespace simd {

    /**
     * Optimized memory copy with SIMD and prefetching.
     * @param from Pointer to the source memory.
     * @param to Pointer to the destination memory.
     * @param count Number of elements to copy (not bytes).
     */
    template<typename T>
    static __forceinline void simdMemCpy(T *__restrict from, T *__restrict to, size_t count) {
#if !defined(__x86_64__) && !defined(_M_X64) && !defined(__i386__) && !defined(_M_IX86)
        memcpy(to, from, count * sizeof(T));
#else
        if ((!IS_AVX512_SUPPORTED && !IS_AVX2_SUPPORTED && !IS_SSE41_SUPPORTED) || count < 8) {
            memcpy(to, from, count * sizeof(T));
        }

        constexpr size_t AVX512_ELEMENTS = AVX512_BLOCK_SIZE / sizeof(T);
        constexpr size_t AVX512_PREFETCH = AVX512_ELEMENTS * 4;
        constexpr size_t AVX2_ELEMENTS = AVX2_BLOCK_SIZE / sizeof(T);
        constexpr size_t AVX2_PREFETCH = AVX2_ELEMENTS * 4;
        constexpr size_t SSE41_ELEMENTS = SSE41_BLOCK_SIZE / sizeof(T);
        constexpr size_t SSE41_PREFETCH = SSE41_ELEMENTS * 4;

        size_t copied = 0;

        // AVX-512 block
        if (IS_AVX512_SUPPORTED && count - copied >= AVX512_ELEMENTS) {
            for (; count - copied >= AVX512_ELEMENTS; copied += AVX512_ELEMENTS) {
                if (count - copied > AVX512_PREFETCH) {
                    prefetchL1(from + copied + AVX512_PREFETCH);
                }

                __m512i vec = _mm512_loadu_si512(from + copied);
                _mm512_storeu_si512(to + copied, vec);
            }
        }

        // AVX2 block
        if (IS_AVX2_SUPPORTED && count - copied >= AVX2_ELEMENTS) {
            for (; count - copied >= AVX2_ELEMENTS; copied += AVX2_ELEMENTS) {
                if (count - copied > AVX2_PREFETCH) {
                    prefetchL1(from + copied + AVX2_PREFETCH);
                }

                __m256i vec = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(from + copied));
                _mm256_storeu_si256(reinterpret_cast<__m256i *>(to + copied), vec);
            }
        }

        // SSE4.1 block
        if (IS_SSE41_SUPPORTED && count - copied >= SSE41_ELEMENTS) {
            for (; count - copied >= SSE41_ELEMENTS; copied += SSE41_ELEMENTS) {
                if (count - copied > SSE41_PREFETCH) {
                    prefetchL1(from + copied + SSE41_PREFETCH);
                }

                __m128i vec = _mm_loadu_si128(reinterpret_cast<const __m128i *>(from + copied));
                _mm_storeu_si128(reinterpret_cast<__m128i *>(to + copied), vec);
            }
        }

        // Handle remaining elements manually (less than the smallest SIMD width)
        while (copied < count) {
            to[copied] = from[copied];
            ++copied;
        }
#endif
    }

    static __forceinline void simdMemCpy(void *__restrict from, void *__restrict to, size_t count) = delete;

    /**
     * Optimized memory copy with SIMD and prefetching.
     * MAKE SURE MEMORY IS ALIGNED with 64 bytes.
     * @param from Pointer to the source memory.
     * @param to Pointer to the destination memory.
     * @param count Number of elements to copy (not bytes).
     */
    template<typename T>
    static __forceinline void simdMemCpyAligned(T *__restrict from, T *__restrict to, size_t count) {
#if !defined(__x86_64__) && !defined(_M_X64) && !defined(__i386__) && !defined(_M_IX86)
        memcpy(to, from, count * sizeof(T));
#else
        if ((!IS_AVX512_SUPPORTED && !IS_AVX2_SUPPORTED && !IS_SSE41_SUPPORTED) || count < 8) {
            memcpy(to, from, count * sizeof(T));
        }

        constexpr size_t AVX512_ELEMENTS = AVX512_BLOCK_SIZE / sizeof(T);
        constexpr size_t AVX512_PREFETCH = AVX512_ELEMENTS * 4;
        constexpr size_t AVX2_ELEMENTS = AVX2_BLOCK_SIZE / sizeof(T);
        constexpr size_t AVX2_PREFETCH = AVX2_ELEMENTS * 4;
        constexpr size_t SSE41_ELEMENTS = SSE41_BLOCK_SIZE / sizeof(T);
        constexpr size_t SSE41_PREFETCH = SSE41_ELEMENTS * 4;

        size_t copied = 0;

        // AVX-512 block
        if (IS_AVX512_SUPPORTED && count - copied >= AVX512_ELEMENTS) {
            for (; count - copied >= AVX512_ELEMENTS; copied += AVX512_ELEMENTS) {
                if (count - copied > AVX512_PREFETCH) {
                    prefetchL1(from + copied + AVX512_PREFETCH);
                }

                __m512i vec = _mm512_load_si512(from + copied);
                _mm512_store_si512(to + copied, vec);
            }
        }

        // AVX2 block
        if (IS_AVX2_SUPPORTED && count - copied >= AVX2_ELEMENTS) {
            for (; count - copied >= AVX2_ELEMENTS; copied += AVX2_ELEMENTS) {
                if (count - copied > AVX2_PREFETCH) {
                    prefetchL1(from + copied + AVX2_PREFETCH);
                }

                __m256i vec = _mm256_load_si256(reinterpret_cast<const __m256i *>(from + copied));
                _mm256_store_si256(reinterpret_cast<__m256i *>(to + copied), vec);
            }
        }

        // SSE4.1 block
        if (IS_SSE41_SUPPORTED && count - copied >= SSE41_ELEMENTS) {
            for (; count - copied >= SSE41_ELEMENTS; copied += SSE41_ELEMENTS) {
                if (count - copied > SSE41_PREFETCH) {
                    prefetchL1(from + copied + SSE41_PREFETCH);
                }

                __m128i vec = _mm_load_si128(reinterpret_cast<const __m128i *>(from + copied));
                _mm_store_si128(reinterpret_cast<__m128i *>(to + copied), vec);
            }
        }

        // Handle remaining elements manually (less than the smallest SIMD width)
        while (copied < count) {
            to[copied] = from[copied];
            ++copied;
        }
#endif
    }

    static __forceinline void simdMemCpyAligned(void *__restrict from, void *__restrict to, size_t count) = delete;

    /**
     * Optimized memory reverse with SIMD and prefetching.
     * @param data Pointer to data memory
     * @param count Number of elements to reverse (not bytes).
     */
    template<typename T>
    static constexpr __forceinline void simdReverse(T *data, size_t count) {
        qReverse<sizeof(T)>(data, count);
    }

    static constexpr __forceinline void simdReverse(void *data, size_t count) = delete;
}
#endif //PYFASTUTIL_SIMD_UTILS_H
