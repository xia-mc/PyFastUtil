//
// Created by xia__mc on 2024/11/11.
//

#ifndef PYFASTUTIL_SIMDHELPER_H
#define PYFASTUTIL_SIMDHELPER_H

#include <cstdlib>
#include "utils/PythonPCH.h"

namespace simd {
    extern const bool IS_AVX2_SUPPORTED;
    extern const bool IS_AVX512_SUPPORTED;
    extern const bool IS_SSE41_SUPPORTED;
    extern const bool IS_SSSE3_SUPPORTED;
    extern const bool IS_ARM_NEON_SUPPORTED;

    // compat
    static constexpr size_t SSE41_BLOCK_SIZE = 16;
    static constexpr size_t AVX2_BLOCK_SIZE = 32;
    static constexpr size_t AVX512_BLOCK_SIZE = 64;

    static constexpr size_t SSE41_INTS = SSE41_BLOCK_SIZE / (sizeof(int));
    static constexpr size_t AVX2_INTS = AVX2_BLOCK_SIZE / (sizeof(int));
    static constexpr size_t AVX512_INTS = AVX512_BLOCK_SIZE / (sizeof(int));

    static constexpr size_t SSE41_LONG_LONGS = SSE41_BLOCK_SIZE / (sizeof(long long));
    static constexpr size_t AVX2_LONG_LONGS = AVX2_BLOCK_SIZE / (sizeof(long long));
    static constexpr size_t AVX512_LONG_LONGS = AVX512_BLOCK_SIZE / (sizeof(long long));

    static constexpr size_t SSE41_PY_OBJECTS = SSE41_BLOCK_SIZE / (sizeof(PyObject *));
    static constexpr size_t AVX2_PY_OBJECTS = AVX2_BLOCK_SIZE / (sizeof(PyObject *));
    static constexpr size_t AVX512_PY_OBJECTS = AVX512_BLOCK_SIZE / (sizeof(PyObject *));

    // Prefetch 4 cache lines ahead
    static constexpr size_t AVX512_PREFETCH_INT = AVX512_INTS * 4;
    static constexpr size_t AVX2_PREFETCH_INT = AVX2_INTS * 4;
    static constexpr size_t SSE41_PREFETCH_INT = SSE41_INTS * 4;

    static constexpr size_t AVX512_PREFETCH_LONG_LONG = AVX512_LONG_LONGS * 4;
    static constexpr size_t AVX2_PREFETCH_LONG_LONG = AVX2_LONG_LONGS * 4;
    static constexpr size_t SSE41_PREFETCH_LONG_LONG = SSE41_LONG_LONGS * 4;

    static constexpr size_t AVX512_PREFETCH_PY_OBJECT = AVX512_PY_OBJECTS * 4;
    static constexpr size_t AVX2_PREFETCH_PY_OBJECT = AVX2_PY_OBJECTS * 4;
    static constexpr size_t SSE41_PREFETCH_PY_OBJECT = SSE41_PY_OBJECTS * 4;
}

#endif //PYFASTUTIL_SIMDHELPER_H
