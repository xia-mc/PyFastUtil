//
// Created by xia__mc on 2024/11/11.
//
#include "BitonicSort.h"

#include <vector>
#include <algorithm>

#if !defined(__arm__) && !defined(__arm64__)

#include <immintrin.h>
#include "utils/simd/Utils.h"

#endif

#include <utils/PythonPCH.h>
#include "SIMD.h"
#include "utils/TimSort.h"
#include "utils/memory/AlignedAllocator.h"
#include "utils/memory/PreFetch.h"

namespace simd {
#if !defined(__arm__) && !defined(__arm64__)
    struct alignas(32) AVX2_MARKS {
        alignas(32) const __m256i M1 = _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0);
        alignas(32) const __m256i M2 = _mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1);
        alignas(32) const __m256i M3 = _mm256_set_epi32(5, 4, 7, 6, 1, 0, 3, 2);
        alignas(32) const __m256i M4 = _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4);
    };
    struct alignas(64) AVX512_MARKS {
        alignas(64) const __m512i M1 = _mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
        alignas(64) const __m512i M2 = _mm512_set_epi32(14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1);
        alignas(64) const __m512i M3 = _mm512_set_epi32(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2);
        alignas(64) const __m512i M4 = _mm512_set_epi32(11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12);
    };

    static AVX2_MARKS *avx2Marks = nullptr;
    static AVX512_MARKS *avx512Marks = nullptr;
#endif

    void init() {
#if !defined(__arm__) && !defined(__arm64__)
        if (IS_AVX2_SUPPORTED) {
            avx2Marks = new AVX2_MARKS();
        }
        if (IS_AVX512_SUPPORTED) {
            avx512Marks = new AVX512_MARKS();
        }
#endif
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "portability-simd-intrinsics"
#pragma ide diagnostic ignored "NullDereference"
#if !defined(__arm__) && !defined(__arm64__)

    __forceinline void sort8Epi32AVX2(__m256i &vec) {
        __m256i swapped = _mm256_permutevar8x32_epi32(vec, avx2Marks->M1);
        vec = _mm256_min_epi32(vec, swapped);

        swapped = _mm256_permutevar8x32_epi32(vec, avx2Marks->M2);
        vec = _mm256_min_epi32(vec, swapped);

        swapped = _mm256_permutevar8x32_epi32(vec, avx2Marks->M3);
        vec = _mm256_min_epi32(vec, swapped);

        swapped = _mm256_permutevar8x32_epi32(vec, avx2Marks->M4);
        vec = _mm256_min_epi32(vec, swapped);
    }

    /**
     * sort 8 nums with avx2 reversed
     */
    __forceinline void sort8Epi32AVX2Reversed(__m256i &vec) {
        __m256i swapped = _mm256_permutevar8x32_epi32(vec, avx2Marks->M1);
        vec = _mm256_max_epi32(vec, swapped);

        swapped = _mm256_permutevar8x32_epi32(vec, avx2Marks->M2);
        vec = _mm256_max_epi32(vec, swapped);

        swapped = _mm256_permutevar8x32_epi32(vec, avx2Marks->M3);
        vec = _mm256_max_epi32(vec, swapped);

        swapped = _mm256_permutevar8x32_epi32(vec, avx2Marks->M4);
        vec = _mm256_max_epi32(vec, swapped);
    }

    /**
     * sort 16 nums with avx512
     */
    __forceinline void sort16Epi32AVX512(__m512i &vec) {
        __m512i swapped = _mm512_permutexvar_epi32(avx512Marks->M1, vec);
        vec = _mm512_min_epi32(vec, swapped);

        swapped = _mm512_permutexvar_epi32(avx512Marks->M2, vec);
        vec = _mm512_min_epi32(vec, swapped);

        swapped = _mm512_permutexvar_epi32(avx512Marks->M3, vec);
        vec = _mm512_min_epi32(vec, swapped);

        swapped = _mm512_permutexvar_epi32(avx512Marks->M4, vec);
        vec = _mm512_min_epi32(vec, swapped);
    }

    /**
     * sort 16 nums with avx512
     */
    __forceinline void sort16Epi32AVX512Reversed(__m512i &vec) {
        __m512i swapped = _mm512_permutexvar_epi32(avx512Marks->M1, vec);
        vec = _mm512_max_epi32(vec, swapped);

        swapped = _mm512_permutexvar_epi32(avx512Marks->M2, vec);
        vec = _mm512_max_epi32(vec, swapped);

        swapped = _mm512_permutexvar_epi32(avx512Marks->M3, vec);
        vec = _mm512_max_epi32(vec, swapped);

        swapped = _mm512_permutexvar_epi32(avx512Marks->M4, vec);
        vec = _mm512_max_epi32(vec, swapped);
    }

    __forceinline void doSingleMerge(const size_t &left, const size_t &mid, const size_t &right,
                                     std::vector<int, AlignedAllocator<int, 64>> &src,
                                     std::vector<int, AlignedAllocator<int, 64>> &dst) {
        size_t i = left, j = mid + 1, k = left;

        // merge 2 sorted blocks with AVX-512
        if (IS_AVX512_SUPPORTED) {
            while (i <= mid - AVX512_INTS + 1 && j <= right - AVX512_INTS + 1) {
                __m512i vec_i = _mm512_load_si512(&(src[i]));
                __m512i vec_j = _mm512_load_si512(&(src[j]));

                __m512i min_vals = _mm512_min_epi32(vec_i, vec_j);

                _mm512_store_si512(&(dst[k]), min_vals);
                k += AVX512_INTS;
                i += AVX512_INTS;
            }
        }

        // merge 2 sorted blocks with AVX2
        if (IS_AVX2_SUPPORTED) {
            while (i <= mid - AVX2_INTS + 1 && j <= right - AVX2_INTS + 1) {
                __m256i vec_i = _mm256_load_si256(reinterpret_cast<__m256i *>(&(src[i])));
                __m256i vec_j = _mm256_load_si256(reinterpret_cast<__m256i *>(&(src[j])));

                __m256i min_vals = _mm256_min_epi32(vec_i, vec_j);

                _mm256_store_si256((__m256i *) &(dst[k]), min_vals);
                k += AVX2_INTS;
                i += AVX2_INTS;
            }
        }

        // merge 2 sorted blocks with SSE4.1
        if (IS_SSE41_SUPPORTED) {
            while (i <= mid - SSE41_INTS + 1 && j <= right - SSE41_INTS + 1) {
                __m128i vec_i = _mm_load_si128((__m128i *) &(src[i]));
                __m128i vec_j = _mm_load_si128((__m128i *) &(src[j]));

                __m128i min_vals = _mm_min_epi32(vec_i, vec_j);

                _mm_store_si128((__m128i *) &(dst[k]), min_vals);
                k += SSE41_INTS;
                i += SSE41_INTS;
            }
        }

        // elements left
        while (i <= mid && j <= right) {
            if (src[i] < src[j]) {
                dst[k++] = src[i++];
            } else {
                dst[k++] = src[j++];
            }
        }

        while (i <= mid) {
            dst[k++] = src[i++];
        }

        while (j <= right) {
            dst[k++] = src[j++];
        }
    }

    __forceinline void doSingleMergeReversed(const size_t &left, const size_t &mid, const size_t &right,
                                             std::vector<int, AlignedAllocator<int, 64>> &src,
                                             std::vector<int, AlignedAllocator<int, 64>> &dst) {
        size_t i = left, j = mid + 1, k = left;

        // merge 2 sorted blocks with AVX-512
        while (IS_AVX512_SUPPORTED && i <= mid - AVX512_INTS + 1 && j <= right - AVX512_INTS + 1) {
            __m512i vec_i = _mm512_load_si512(&(src[i]));
            __m512i vec_j = _mm512_load_si512(&(src[j]));

            __m512i max_vals = _mm512_max_epi32(vec_i, vec_j);

            _mm512_store_si512(&(dst[k]), max_vals);
            k += AVX512_INTS;
            j += AVX512_INTS;
        }

        // merge 2 sorted blocks with AVX2
        while (IS_AVX2_SUPPORTED && i <= mid - AVX2_INTS + 1 && j <= right - AVX2_INTS + 1) {
            __m256i vec_i = _mm256_load_si256(reinterpret_cast<__m256i *>(&(src[i])));
            __m256i vec_j = _mm256_load_si256(reinterpret_cast<__m256i *>(&(src[j])));

            __m256i max_vals = _mm256_max_epi32(vec_i, vec_j);

            _mm256_store_si256((__m256i *) &(dst[k]), max_vals);
            k += AVX2_INTS;
            j += AVX2_INTS;
        }

        // merge 2 sorted blocks with SSE4.1
        while (IS_SSE41_SUPPORTED && i <= mid - SSE41_INTS + 1 && j <= right - SSE41_INTS + 1) {
            __m128i vec_i = _mm_load_si128((__m128i *) &(src[i]));
            __m128i vec_j = _mm_load_si128((__m128i *) &(src[j]));

            __m128i max_vals = _mm_max_epi32(vec_i, vec_j);

            _mm_store_si128((__m128i *) &(dst[k]), max_vals);
            k += SSE41_INTS;
            j += SSE41_INTS;
        }

        // elements left
        while (i <= mid && j <= right) {
            if (src[i] > src[j]) {
                dst[k++] = src[i++];
            } else {
                dst[k++] = src[j++];
            }
        }

        while (i <= mid) {
            dst[k++] = src[i++];
        }

        while (j <= right) {
            dst[k++] = src[j++];
        }
    }

    /**
     * Merge sorted blocks with SIMD optimization, or fallback
     * make sure aligned
     */
    __forceinline void mergeSortedBlocks(std::vector<int, AlignedAllocator<int, 64>> &data, const size_t &blockSize) {
        const size_t total = data.size();
        auto temp = std::vector<int, AlignedAllocator<int, 64>>(total);

        bool cycle = true;
        size_t mid;
        size_t right;
        for (size_t size = blockSize; size < total; size *= 2) {
            size_t left = 0;
            prefetchL1(&data + left);
            prefetchL1(&temp + left);
            for (; left < total; left += 2 * size) {
                mid = std::min(left + size - 1, total - 1);
                right = std::min(left + 2 * size - 1, total - 1);
                if (cycle) {
                    doSingleMerge(left, mid, right, data, temp);
                } else {
                    doSingleMerge(left, mid, right, temp, data);
                }
            }

            cycle = !cycle;
        }

        // copy the final result
        if (!cycle) {
            simdMemCpyAligned(temp.data(), data.data(), temp.size());
        }
    }

    /**
     * Merge sorted blocks with SIMD optimization reversed, or fallback
     */
    __forceinline void
    mergeSortedBlocksReversed(std::vector<int, AlignedAllocator<int, 64>> &data, const size_t &block_size) {
        const size_t total = data.size();
        auto temp = std::vector<int, AlignedAllocator<int, 64>>(total);

        bool cycle = true;
        for (size_t size = block_size; size < total; size *= 2) {
            for (size_t left = 0; left < total; left += 2 * size) {
                if (cycle) {
                    doSingleMergeReversed(total, size, left, data, temp);
                } else {
                    doSingleMergeReversed(total, size, left, temp, data);
                }
            }

            cycle = !cycle;
        }

        // copy the final result
        if (!cycle) {
            simdMemCpyAligned(temp.data(), data.data(), temp.size());
        }
    }

#endif
#pragma clang diagnostic pop

    /**
     * Try to sort with simd optimize, or fallback if unsupported
     * MAKE SURE VECTOR IS ALIGNED!
     * @param vector vector to sort
     */
    void simdsort(std::vector<int, AlignedAllocator<int, 64>> &vector, const bool &reverse) {
        const auto size = vector.size();
        if (size <= 1) return;

        const auto begin = vector.begin();
        const auto end = vector.end();

        if (size < 8) {
            if (reverse) {
                std::sort(begin, end, std::greater<>());
            } else {
                std::sort(begin, end);
            }
            return;
        }

#if defined(__arm__) || defined(__arm64__)
        if (size > 5000) {
            if (reverse) {
                gfx::timsort(begin, end, std::greater<>());
            } else {
                gfx::timsort(begin, end);
            }
        } else {
            if (reverse) {
                std::sort(begin, end, std::greater<>());
            } else {
                std::sort(begin, end);
            }
        }
#else
        if (!IS_AVX2_SUPPORTED && !IS_AVX512_SUPPORTED) {
            // fallback
            if (size > 5000) {
                if (reverse) {
                    gfx::timsort(begin, end, std::greater<>());
                } else {
                    gfx::timsort(begin, end);
                }
            } else {
                if (reverse) {
                    std::sort(begin, end, std::greater<>());
                } else {
                    std::sort(begin, end);
                }
            }
            return;
        }

        // get pointer to do simd
        int *data = &(*begin);
        size_t sortedCount = 0;
        size_t minBlockSize = AVX512_INTS;

        constexpr size_t AVX512_PREFETCH_DISTANCE = AVX512_INTS * 4; // Prefetch 4 cache lines ahead
        constexpr size_t AVX2_PREFETCH_DISTANCE = AVX2_INTS * 4;     // Prefetch 4 cache lines ahead

        if (IS_AVX512_SUPPORTED && sortedCount + AVX512_INTS < size) {
            // simd sort with avx512
            prefetchL1(avx512Marks);
            for (; sortedCount + AVX512_INTS <= size; sortedCount += AVX512_INTS) {
                if (size - sortedCount > AVX512_PREFETCH_DISTANCE) {
                    prefetchL1(data + sortedCount + AVX512_PREFETCH_DISTANCE);
                }

                __m512i vec = _mm512_load_si512(data + sortedCount);  // load
                if (reverse) {  // sort
                    sort16Epi32AVX512Reversed(vec);
                } else {
                    sort16Epi32AVX512(vec);
                }
                _mm512_store_si512(data + sortedCount, vec);  // store
            }
        }

        if (IS_AVX2_SUPPORTED && sortedCount + AVX2_INTS < size) {
            // simd sort with avx2
            prefetchL1(avx2Marks);
            for (; sortedCount + AVX2_INTS <= size; sortedCount += AVX2_INTS) {
                if (size - sortedCount > AVX2_PREFETCH_DISTANCE) {
                    prefetchL1(data + sortedCount + AVX2_PREFETCH_DISTANCE);
                }

                __m256i vec = _mm256_load_si256((__m256i *) (data + sortedCount));  // load
                if (reverse) {  // sort
                    sort8Epi32AVX2Reversed(vec);
                } else {
                    sort8Epi32AVX2(vec);
                }
                _mm256_store_si256((__m256i *) (data + sortedCount), vec);  // store
            }
            minBlockSize = 8;
        }

        if (sortedCount < size) {
            if (reverse) {
                std::sort(begin + static_cast<std::vector<int>::difference_type>(sortedCount), end,
                          std::greater<>());
            } else {
                std::sort(begin + static_cast<std::vector<int>::difference_type>(sortedCount), end);
            }
        }

        // merge all sorted blocks
        if (reverse)
            mergeSortedBlocksReversed(vector, minBlockSize);
        else
            mergeSortedBlocks(vector, minBlockSize);
#endif
    }
}
