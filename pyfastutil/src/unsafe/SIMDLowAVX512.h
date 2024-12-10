//
// Created by xia__mc on 2024/12/9.
//

#ifndef PYFASTUTIL_SIMDLOWAVX512_H
#define PYFASTUTIL_SIMDLOWAVX512_H

#include "utils/PythonPCH.h"

typedef struct SIMDLowAVX512 {
    PyObject_HEAD;
} SIMDLowAVX512;

PyMODINIT_FUNC PyInit_SIMDLowAVX512();

#endif //PYFASTUTIL_SIMDLOWAVX512_H
