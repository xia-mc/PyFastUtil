//
// Created by xia__mc on 2024/11/12.
//

#ifndef PYFASTUTIL_UTILS_H
#define PYFASTUTIL_UTILS_H

#include <immintrin.h>
#include "SIMD.h"
#include "utils/memory/PreFetch.h"
#include "Compat.h"

namespace simd {
    constexpr size_t AVX512_PREFETCH_DISTANCE = AVX512_INTS * 4; // Prefetch 4 cache lines ahead
    constexpr size_t AVX2_PREFETCH_DISTANCE = AVX2_INTS * 4;     // Prefetch 4 cache lines ahead
    constexpr size_t SSE41_PREFETCH_DISTANCE = SSE41_INTS * 4;   // Prefetch 4 cache lines ahead

    /**
     * Optimized memory copy with SIMD and prefetching.
     * @param from Pointer to the source memory.
     * @param to Pointer to the destination memory.
     * @param count Number of elements to copy (not bytes).
     */
    static inline void simdMemcpy(int *from, int *to, size_t count) {
        if ((!IS_AVX512_SUPPORTED && !IS_AVX2_SUPPORTED && !IS_SSE41_SUPPORTED) || count < 8) {
            memcpy(to, from, count * sizeof(int));
        }

        size_t copied = 0;

        // AVX-512 block
        if (IS_AVX512_SUPPORTED && count - copied >= AVX512_INTS) {
            for (; count - copied >= AVX512_INTS; copied += AVX512_INTS) {
                if (count - copied > AVX512_PREFETCH_DISTANCE) {
                    prefetchL1(from + copied + AVX512_PREFETCH_DISTANCE);
                }

                __m512i vec = _mm512_loadu_si512(from + copied);
                _mm512_storeu_si512(to + copied, vec);
            }
        }

        // AVX2 block
        if (IS_AVX2_SUPPORTED && count - copied >= AVX2_INTS) {
            for (; count - copied >= AVX2_INTS; copied += AVX2_INTS) {
                if (count - copied > AVX2_PREFETCH_DISTANCE) {
                    prefetchL1(from + copied + AVX2_PREFETCH_DISTANCE);
                }

                __m256i vec = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(from + copied));
                _mm256_storeu_si256(reinterpret_cast<__m256i *>(to + copied), vec);
            }
        }

        // SSE4.1 block
        if (IS_SSE41_SUPPORTED && count - copied >= SSE41_INTS) {
            for (; count - copied >= SSE41_INTS; copied += SSE41_INTS) {
                if (count - copied > SSE41_PREFETCH_DISTANCE) {
                    prefetchL1(from + copied + SSE41_PREFETCH_DISTANCE);
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
    }

    /**
     * Optimized memory copy with SIMD and prefetching.
     * MAKE SURE MEMORY IS ALIGNED with 64 bytes.
     * @param from Pointer to the source memory.
     * @param to Pointer to the destination memory.
     * @param count Number of elements to copy (not bytes).
     */
    static inline void simdMemCpyAligned(int *from, int *to, size_t count) {
        if ((!IS_AVX512_SUPPORTED && !IS_AVX2_SUPPORTED && !IS_SSE41_SUPPORTED) || count < 8) {
            memcpy(to, from, count * sizeof(int));
        }

        size_t copied = 0;

        // AVX-512 block
        if (IS_AVX512_SUPPORTED && count - copied >= AVX512_INTS) {
            for (; count - copied >= AVX512_INTS; copied += AVX512_INTS) {
                if (count - copied > AVX512_PREFETCH_DISTANCE) {
                    prefetchL1(from + copied + AVX512_PREFETCH_DISTANCE);
                }

                __m512i vec = _mm512_load_si512(from + copied);
                _mm512_store_si512(to + copied, vec);
            }
        }

        // AVX2 block
        if (IS_AVX2_SUPPORTED && count - copied >= AVX2_INTS) {
            for (; count - copied >= AVX2_INTS; copied += AVX2_INTS) {
                if (count - copied > AVX2_PREFETCH_DISTANCE) {
                    prefetchL1(from + copied + AVX2_PREFETCH_DISTANCE);
                }

                __m256i vec = _mm256_load_si256(reinterpret_cast<const __m256i *>(from + copied));
                _mm256_store_si256(reinterpret_cast<__m256i *>(to + copied), vec);
            }
        }

        // SSE4.1 block
        if (IS_SSE41_SUPPORTED && count - copied >= SSE41_INTS) {
            for (; count - copied >= SSE41_INTS; copied += SSE41_INTS) {
                if (count - copied > SSE41_PREFETCH_DISTANCE) {
                    prefetchL1(from + copied + SSE41_PREFETCH_DISTANCE);
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
    }

    /**
     * Optimized memory reverse with SIMD and prefetching.
     * @param data Pointer to data memory
     * @param count Number of elements to reverse (not bytes).
     */
    static inline void simdReverse(int *data, size_t count) {
        if ((!IS_AVX512_SUPPORTED && !IS_AVX2_SUPPORTED && !IS_SSE41_SUPPORTED) || count < 8) {
            std::reverse(data, data + count);
            return;
        }

        size_t processed = 0;

        if (IS_AVX512_SUPPORTED && count - processed >= AVX512_INTS * 2) {
            for (; count - processed >= AVX512_INTS * 2; processed += AVX512_INTS * 2) {
                if (count - processed > AVX512_PREFETCH_DISTANCE) {
                    prefetchL1(data + processed + AVX512_PREFETCH_DISTANCE);
                }

                auto left = _mm512_loadu_si512(data + processed);
                auto right = _mm512_loadu_si512(data + count - processed - AVX512_INTS);

                _mm512_storeu_si512(data + processed, right);
                _mm512_storeu_si512(data + count - processed - AVX512_INTS, left);
            }
        }

        if (IS_AVX2_SUPPORTED && count - processed >= AVX2_INTS * 2) {
            for (; count - processed >= AVX2_INTS * 2; processed += AVX2_INTS * 2) {
                if (count - processed > AVX2_PREFETCH_DISTANCE) {
                    prefetchL1(data + processed + AVX2_PREFETCH_DISTANCE);
                }

                auto left = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(data + processed));
                auto right = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(data + count - processed - AVX2_INTS));

                _mm256_storeu_si256(reinterpret_cast<__m256i *>(data + processed), right);
                _mm256_storeu_si256(reinterpret_cast<__m256i *>(data + count - processed - AVX2_INTS), left);
            }
        }

        if (IS_SSE41_SUPPORTED && count - processed >= SSE41_INTS * 2) {
            for (; count - processed >= SSE41_INTS * 2; processed += SSE41_INTS * 2) {
                if (count - processed > SSE41_PREFETCH_DISTANCE) {
                    prefetchL1(data + processed + SSE41_PREFETCH_DISTANCE);
                }

                auto left = _mm_loadu_si128(reinterpret_cast<const __m128i *>(data + processed));
                auto right = _mm_loadu_si128(reinterpret_cast<const __m128i *>(data + count - processed - SSE41_INTS));

                _mm_storeu_si128(reinterpret_cast<__m128i *>(data + processed), right);
                _mm_storeu_si128(reinterpret_cast<__m128i *>(data + count - processed - SSE41_INTS), left);
            }
        }

        if (count - processed > 1) {
            std::reverse(data + processed, data + count - processed);
        }
    }

    /**
     * Optimized memory reverse with SIMD and prefetching.
     * MAKE SURE MEMORY IS ALIGNED with 64 bytes.
     * @param data Pointer to data memory
     * @param count Number of elements to reverse (not bytes).
     */
    static inline void simdReverseAligned(int *data, size_t count) {
        if ((!IS_AVX512_SUPPORTED && !IS_AVX2_SUPPORTED && !IS_SSE41_SUPPORTED) || count < 8) {
            std::reverse(data, data + count);
            return;
        }

        size_t processed = 0;

        if (IS_AVX512_SUPPORTED && count - processed >= AVX512_INTS * 2) {
            for (; count - processed >= AVX512_INTS * 2; processed += AVX512_INTS * 2) {
                if (count - processed > AVX512_PREFETCH_DISTANCE) {
                    prefetchL1(data + processed + AVX512_PREFETCH_DISTANCE);
                }

                auto left = _mm512_load_si512(data + processed);
                auto right = _mm512_load_si512(data + count - processed - AVX512_INTS);

                _mm512_store_si512(data + processed, right);
                _mm512_store_si512(data + count - processed - AVX512_INTS, left);
            }
        }

        if (IS_AVX2_SUPPORTED && count - processed >= AVX2_INTS * 2) {
            for (; count - processed >= AVX2_INTS * 2; processed += AVX2_INTS * 2) {
                if (count - processed > AVX2_PREFETCH_DISTANCE) {
                    prefetchL1(data + processed + AVX2_PREFETCH_DISTANCE);
                }

                auto left = _mm256_load_si256(reinterpret_cast<const __m256i *>(data + processed));
                auto right = _mm256_load_si256(reinterpret_cast<const __m256i *>(data + count - processed - AVX2_INTS));

                _mm256_store_si256(reinterpret_cast<__m256i *>(data + processed), right);
                _mm256_store_si256(reinterpret_cast<__m256i *>(data + count - processed - AVX2_INTS), left);
            }
        }

        if (IS_SSE41_SUPPORTED && count - processed >= SSE41_INTS * 2) {
            for (; count - processed >= SSE41_INTS * 2; processed += SSE41_INTS * 2) {
                if (count - processed > SSE41_PREFETCH_DISTANCE) {
                    prefetchL1(data + processed + SSE41_PREFETCH_DISTANCE);
                }

                auto left = _mm_load_si128(reinterpret_cast<const __m128i *>(data + processed));
                auto right = _mm_load_si128(reinterpret_cast<const __m128i *>(data + count - processed - SSE41_INTS));

                _mm_store_si128(reinterpret_cast<__m128i *>(data + processed), right);
                _mm_store_si128(reinterpret_cast<__m128i *>(data + count - processed - SSE41_INTS), left);
            }
        }

        if (count - processed > 1) {
            std::reverse(data + processed, data + count - processed);
        }
    }
}
#endif //PYFASTUTIL_UTILS_H
