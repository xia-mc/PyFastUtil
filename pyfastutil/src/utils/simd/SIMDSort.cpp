//
// Created by xia__mc on 2024/11/11.
//
#include "SIMDSort.h"

#include <vector>
#include <algorithm>
#include <immintrin.h>
#include "SIMD.h"
#include "utils/TimSort.h"

namespace simd {

#pragma clang diagnostic push
#pragma ide diagnostic ignored "portability-simd-intrinsics"

    /**
     * Sort 4 int32 with AVX2
     */
    __forceinline void sort4_epi32_avx2(__m256i &vec4_epi32, const bool &reverse) {
        __m256i shuffled_vec;

        shuffled_vec = _mm256_shuffle_epi32(vec4_epi32, _MM_SHUFFLE(3, 2, 1, 0));
        if (reverse) {
            vec4_epi32 = _mm256_max_epi32(vec4_epi32, shuffled_vec);
            shuffled_vec = _mm256_min_epi32(vec4_epi32, shuffled_vec);
        } else {
            vec4_epi32 = _mm256_min_epi32(vec4_epi32, shuffled_vec);
            shuffled_vec = _mm256_max_epi32(vec4_epi32, shuffled_vec);
        }
        vec4_epi32 = _mm256_blend_epi32(vec4_epi32, shuffled_vec, 0b01010101);

        shuffled_vec = _mm256_shuffle_epi32(vec4_epi32, _MM_SHUFFLE(2, 3, 0, 1));
        if (reverse) {
            vec4_epi32 = _mm256_max_epi32(vec4_epi32, shuffled_vec);
            shuffled_vec = _mm256_min_epi32(vec4_epi32, shuffled_vec);
        } else {
            vec4_epi32 = _mm256_min_epi32(vec4_epi32, shuffled_vec);
            shuffled_vec = _mm256_max_epi32(vec4_epi32, shuffled_vec);
        }
        vec4_epi32 = _mm256_blend_epi32(vec4_epi32, shuffled_vec, 0b00110011);

        shuffled_vec = _mm256_shuffle_epi32(vec4_epi32, _MM_SHUFFLE(1, 0, 3, 2));
        if (reverse) {
            vec4_epi32 = _mm256_max_epi32(vec4_epi32, shuffled_vec);
            shuffled_vec = _mm256_min_epi32(vec4_epi32, shuffled_vec);
        } else {
            vec4_epi32 = _mm256_min_epi32(vec4_epi32, shuffled_vec);
            shuffled_vec = _mm256_max_epi32(vec4_epi32, shuffled_vec);
        }
        vec4_epi32 = _mm256_blend_epi32(vec4_epi32, shuffled_vec, 0b00001111);
    }

    /**
     * Sort 16 int32 with AVX512
     */
    __forceinline void sort16_epi32_avx512(__m512i &vec16_epi32, const bool &reverse) {
        __m512i tmp;

        tmp = _mm512_permutexvar_epi32(_mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0),
                                       vec16_epi32);
        if (reverse) {
            vec16_epi32 = _mm512_max_epi32(vec16_epi32, tmp);
            tmp = _mm512_min_epi32(vec16_epi32, tmp);
        } else {
            vec16_epi32 = _mm512_min_epi32(vec16_epi32, tmp);
            tmp = _mm512_max_epi32(vec16_epi32, tmp);
        }
        vec16_epi32 = _mm512_mask_blend_epi32(0xAAAA, vec16_epi32, tmp); // Blend with mask 0xAAAA (1010101010101010)

        tmp = _mm512_permutexvar_epi32(_mm512_set_epi32(14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1),
                                       vec16_epi32);
        if (reverse) {
            vec16_epi32 = _mm512_max_epi32(vec16_epi32, tmp);
            tmp = _mm512_min_epi32(vec16_epi32, tmp);
        } else {
            vec16_epi32 = _mm512_min_epi32(vec16_epi32, tmp);
            tmp = _mm512_max_epi32(vec16_epi32, tmp);
        }
        vec16_epi32 = _mm512_mask_blend_epi32(0xCCCC, vec16_epi32, tmp); // Blend with mask 0xCCCC (1100110011001100)

        tmp = _mm512_permutexvar_epi32(_mm512_set_epi32(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2),
                                       vec16_epi32);
        if (reverse) {
            vec16_epi32 = _mm512_max_epi32(vec16_epi32, tmp);
            tmp = _mm512_min_epi32(vec16_epi32, tmp);
        } else {
            vec16_epi32 = _mm512_min_epi32(vec16_epi32, tmp);
            tmp = _mm512_max_epi32(vec16_epi32, tmp);
        }
        vec16_epi32 = _mm512_mask_blend_epi32(0xF0F0, vec16_epi32, tmp); // Blend with mask 0xF0F0 (1111000011110000)

        tmp = _mm512_permutexvar_epi32(_mm512_set_epi32(11, 10, 9, 8, 15, 14, 13, 12, 3, 2, 1, 0, 7, 6, 5, 4),
                                       vec16_epi32);
        if (reverse) {
            vec16_epi32 = _mm512_max_epi32(vec16_epi32, tmp);
            tmp = _mm512_min_epi32(vec16_epi32, tmp);
        } else {
            vec16_epi32 = _mm512_min_epi32(vec16_epi32, tmp);
            tmp = _mm512_max_epi32(vec16_epi32, tmp);
        }
        vec16_epi32 = _mm512_mask_blend_epi32(0xFF00, vec16_epi32, tmp); // Blend with mask 0xFF00 (1111111100000000)

        tmp = _mm512_permutexvar_epi32(_mm512_set_epi32(7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8),
                                       vec16_epi32);
        if (reverse) {
            vec16_epi32 = _mm512_max_epi32(vec16_epi32, tmp);
            tmp = _mm512_min_epi32(vec16_epi32, tmp);
        } else {
            vec16_epi32 = _mm512_min_epi32(vec16_epi32, tmp);
            tmp = _mm512_max_epi32(vec16_epi32, tmp);
        }
        vec16_epi32 = _mm512_mask_blend_epi32(0xFFFF, vec16_epi32, tmp); // Final blend with mask 0xFFFF (all elements)
    }

#pragma clang diagnostic pop

    /**
     * Sort int32 with SIMD, or fallback to std::sort/timsort if unsupported
     */
    void simd_sort(std::vector<int>::iterator begin, std::vector<int>::iterator end, const bool &reverse) {
        size_t size = std::distance(begin, end);
        if (size <= 1) return;
        if (!simd::IS_AVX2_SUPPORTED && !simd::IS_AVX512_SUPPORTED) {
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
        }

        int *data = &(*begin);

        // If size >= 16 and AVX512 is supported, use AVX512
        if (simd::IS_AVX512_SUPPORTED && size >= 16) {
            for (size_t i = 0; i + 16 <= size; i += 16) {
                __m512i vec16_epi32 = _mm512_loadu_si512((void *) &data[i]);  // Load 16 int32 elements
                sort16_epi32_avx512(vec16_epi32, reverse);                  // Sort using AVX512
                _mm512_storeu_si512((void *) &data[i], vec16_epi32);          // Store back the sorted values
            }
            // Update the pointer to the remaining elements
            data += (size / 16) * 16;
            size = size % 16;  // Remaining elements after AVX512 processing
        }

        // If size >= 4 and AVX2 is supported, use AVX2
        if (simd::IS_AVX2_SUPPORTED && size >= 4) {
            for (size_t i = 0; i + 4 <= size; i += 4) {
                __m256i vec4_epi32 = _mm256_loadu_si256((__m256i *) &data[i]);  // Load 4 int32 elements
                sort4_epi32_avx2(vec4_epi32, reverse);                        // Sort using AVX2
                _mm256_storeu_si256((__m256i *) &data[i], vec4_epi32);          // Store back the sorted values
            }
            // Update the pointer to the remaining elements
            data += (size / 4) * 4;
            size = size % 4;  // Remaining elements after AVX2 processing
        }

        // Use std::sort for any remaining elements
        if (size > 0) {
            if (reverse) {
                std::sort(data, data + size, std::greater<>());
            } else {
                std::sort(data, data + size);
            }
        }
    }

}