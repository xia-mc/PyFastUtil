//
// Created by xia__mc on 2024/11/11.
//

#ifndef PYFASTUTIL_SIMD_H
#define PYFASTUTIL_SIMD_H

#include <cstdlib>
#include "utils/PythonPCH.h"

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

    const size_t SSE41_LONG_LONGS = SSE41_BLOCK_SIZE / (8 * sizeof(long long));
    const size_t AVX2_LONG_LONGS = AVX2_BLOCK_SIZE / (8 * sizeof(long long));
    const size_t AVX512_LONG_LONGS = AVX512_BLOCK_SIZE / (8 * sizeof(long long));

    const size_t SSE41_PY_OBJECTS = SSE41_BLOCK_SIZE / (8 * sizeof(PyObject *));
    const size_t AVX2_PY_OBJECTS = AVX2_BLOCK_SIZE / (8 * sizeof(PyObject *));
    const size_t AVX512_PY_OBJECTS = AVX512_BLOCK_SIZE / (8 * sizeof(PyObject *));
}

#endif //PYFASTUTIL_SIMD_H
