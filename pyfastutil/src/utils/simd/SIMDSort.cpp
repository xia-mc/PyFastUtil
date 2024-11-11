//
// Created by xia__mc on 2024/11/11.
//
#include "SIMDSort.h"

#include <vector>
#include <algorithm>
#include <immintrin.h>
#include <utils/PythonPCH.h>
#include "SIMD.h"
#include "utils/TimSort.h"
#include "utils/memory/AlignedAllocator.h"

namespace simd {
    constexpr size_t SSE41_BLOCK_SIZE = 4;
    constexpr size_t AVX2_BLOCK_SIZE = 8;
    constexpr size_t AVX512_BLOCK_SIZE = 16;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "portability-simd-intrinsics"

    /**
     * sort 8 nums with avx2
     */
    __forceinline void sort8_epi32_avx2(__m256i &vec, bool reverse) {
        __m256i swapped = _mm256_permutevar8x32_epi32(vec, _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0));
        __m256i minVals = _mm256_min_epi32(vec, swapped);
        __m256i maxVals = _mm256_max_epi32(vec, swapped);
        vec = reverse ? maxVals : minVals;

        swapped = _mm256_permutevar8x32_epi32(vec, _mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1));
        minVals = _mm256_min_epi32(vec, swapped);
        maxVals = _mm256_max_epi32(vec, swapped);
        vec = reverse ? maxVals : minVals;

        swapped = _mm256_permutevar8x32_epi32(vec, _mm256_set_epi32(5, 4, 7, 6, 1, 0, 3, 2));
        minVals = _mm256_min_epi32(vec, swapped);
        maxVals = _mm256_max_epi32(vec, swapped);
        vec = reverse ? maxVals : minVals;

        swapped = _mm256_permutevar8x32_epi32(vec, _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4));
        minVals = _mm256_min_epi32(vec, swapped);
        maxVals = _mm256_max_epi32(vec, swapped);
        vec = reverse ? maxVals : minVals;
    }

    /**
     * sort 16 nums with avx512
     */
    __forceinline void sort16_epi32_avx512(__m512i &vec, bool reverse) {
        __m512i swapped = _mm512_permutexvar_epi32(_mm512_set_epi32(
                15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0), vec);
        __m512i minVals = _mm512_min_epi32(vec, swapped);
        __m512i maxVals = _mm512_max_epi32(vec, swapped);
        vec = reverse ? maxVals : minVals;

        swapped = _mm512_permutexvar_epi32(_mm512_set_epi32(
                14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1), vec);
        minVals = _mm512_min_epi32(vec, swapped);
        maxVals = _mm512_max_epi32(vec, swapped);
        vec = reverse ? maxVals : minVals;

        swapped = _mm512_permutexvar_epi32(_mm512_set_epi32(
                13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2), vec);
        minVals = _mm512_min_epi32(vec, swapped);
        maxVals = _mm512_max_epi32(vec, swapped);
        vec = reverse ? maxVals : minVals;

        swapped = _mm512_permutexvar_epi32(_mm512_set_epi32(
                11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12), vec);
        minVals = _mm512_min_epi32(vec, swapped);
        maxVals = _mm512_max_epi32(vec, swapped);
        vec = reverse ? maxVals : minVals;
    }

    /**
     * Merge sorted blocks with SIMD optimization, or fallback
     */
    __forceinline void
    mergeSortedBlocks(std::vector<int, AlignedAllocator<int, 64>> &data, size_t block_size, bool reverse) {
        size_t n = data.size();
        auto temp = std::vector<int, AlignedAllocator<int, 64>>(data.size());

        auto *src = &data;
        auto *dst = &temp;

        for (size_t size = block_size; size < n; size *= 2) {
            for (size_t left = 0; left < n; left += 2 * size) {
                size_t mid = std::min(left + size - 1, n - 1);
                size_t right = std::min(left + 2 * size - 1, n - 1);

                size_t i = left, j = mid + 1, k = left;

                // merge 2 sorted blocks with AVX-512
                while (IS_AVX512_SUPPORTED && i <= mid - AVX512_BLOCK_SIZE + 1 && j <= right - AVX512_BLOCK_SIZE + 1) {
                    __m512i vec_i = _mm512_load_si512(&((*src)[i]));
                    __m512i vec_j = _mm512_load_si512(&((*src)[j]));

                    __m512i min_vals = _mm512_min_epi32(vec_i, vec_j);
                    __m512i max_vals = _mm512_max_epi32(vec_i, vec_j);

                    if (!reverse) {
                        _mm512_store_si512(&((*dst)[k]), min_vals);
                        k += AVX512_BLOCK_SIZE;
                        i += AVX512_BLOCK_SIZE;
                    } else {
                        _mm512_store_si512(&((*dst)[k]), max_vals);
                        k += AVX512_BLOCK_SIZE;
                        j += AVX512_BLOCK_SIZE;
                    }
                }

                // merge 2 sorted blocks with AVX2
                while (IS_AVX2_SUPPORTED && i <= mid - AVX2_BLOCK_SIZE + 1 && j <= right - AVX2_BLOCK_SIZE + 1) {
                    __m256i vec_i = _mm256_load_si256(reinterpret_cast<__m256i *>(&((*src)[i])));
                    __m256i vec_j = _mm256_load_si256(reinterpret_cast<__m256i *>(&((*src)[j])));

                    __m256i min_vals = _mm256_min_epi32(vec_i, vec_j);
                    __m256i max_vals = _mm256_max_epi32(vec_i, vec_j);

                    if (!reverse) {
                        _mm256_store_si256((__m256i *) &((*dst)[k]), min_vals);
                        k += AVX2_BLOCK_SIZE;
                        i += AVX2_BLOCK_SIZE;
                    } else {
                        _mm256_store_si256((__m256i *) &((*dst)[k]), max_vals);
                        k += AVX2_BLOCK_SIZE;
                        j += AVX2_BLOCK_SIZE;
                    }
                }

                // merge 2 sorted blocks with SSE4.1
                while (IS_SSE41_SUPPORTED && i <= mid - SSE41_BLOCK_SIZE + 1 && j <= right - SSE41_BLOCK_SIZE + 1) {
                    __m128i vec_i = _mm_load_si128((__m128i *) &((*src)[i]));
                    __m128i vec_j = _mm_load_si128((__m128i *) &((*src)[j]));

                    __m128i min_vals = _mm_min_epi32(vec_i, vec_j);
                    __m128i max_vals = _mm_max_epi32(vec_i, vec_j);

                    if (!reverse) {
                        _mm_store_si128((__m128i *) &((*dst)[k]), min_vals);
                        k += SSE41_BLOCK_SIZE;
                        i += SSE41_BLOCK_SIZE;
                    } else {
                        _mm_store_si128((__m128i *) &((*dst)[k]), max_vals);
                        k += SSE41_BLOCK_SIZE;
                        j += SSE41_BLOCK_SIZE;
                    }
                }

                // elements left
                while (i <= mid && j <= right) {
                    if ((!reverse && (*src)[i] < (*src)[j]) || (reverse && (*src)[i] > (*src)[j])) {
                        (*dst)[k++] = (*src)[i++];
                    } else {
                        (*dst)[k++] = (*src)[j++];
                    }
                }

                while (i <= mid) {
                    (*dst)[k++] = (*src)[i++];
                }

                while (j <= right) {
                    (*dst)[k++] = (*src)[j++];
                }
            }

            // data for next merge
            std::swap(src, dst);
        }

        // copy the final result
        if (src != &data) {
            std::copy(temp.begin(), temp.end(), data.begin());
        }
    }

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
        size_t minBlockSize = AVX512_BLOCK_SIZE;

        if (IS_AVX512_SUPPORTED && sortedCount + AVX512_BLOCK_SIZE < size) {
            // simd sort with avx512
            for (; sortedCount + AVX512_BLOCK_SIZE <= size; sortedCount += AVX512_BLOCK_SIZE) {
                __m512i vec = _mm512_load_si512(data + sortedCount);  // load
                sort16_epi32_avx512(vec, reverse);  // sort
                _mm512_store_si512(data + sortedCount, vec);  // store
            }
        }

        if (IS_AVX2_SUPPORTED && sortedCount + AVX2_BLOCK_SIZE < size) {
            // simd sort with avx2
            for (; sortedCount + AVX2_BLOCK_SIZE <= size; sortedCount += AVX2_BLOCK_SIZE) {
                __m256i vec = _mm256_load_si256((__m256i *) (data + sortedCount));  // load
                sort8_epi32_avx2(vec, reverse);  // sort
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
        mergeSortedBlocks(vector, minBlockSize, reverse);
    }
}