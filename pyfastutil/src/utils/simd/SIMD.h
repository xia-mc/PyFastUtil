//
// Created by xia__mc on 2024/11/11.
//

#ifndef PYFASTUTIL_SIMD_H
#define PYFASTUTIL_SIMD_H

#include <cstdlib>

namespace simd {
    extern const bool IS_AVX2_SUPPORTED;
    extern const bool IS_AVX512_SUPPORTED;
    extern const bool IS_SSE41_SUPPORTED;

    constexpr size_t SSE41_BLOCK_SIZE = 128;
    constexpr size_t AVX2_BLOCK_SIZE = 256;
    constexpr size_t AVX512_BLOCK_SIZE = 512;

    const size_t SSE41_INTS = SSE41_BLOCK_SIZE / (8 * sizeof(int));
    const size_t AVX2_INTS = AVX2_BLOCK_SIZE / (8 * sizeof(int));
    const size_t AVX512_INTS = AVX512_BLOCK_SIZE / (8 * sizeof(int));
}

#endif //PYFASTUTIL_SIMD_H
