//
// Created by xia__mc on 2024/12/8.
//

#ifndef PYFASTUTIL_SIMD_H
#define PYFASTUTIL_SIMD_H

#include "utils/PythonPCH.h"

extern "C" {
typedef struct SIMD {
    PyObject_HEAD;
} SIMD;
}

PyMODINIT_FUNC PyInit_SIMD();

#endif //PYFASTUTIL_SIMDHELPER_H
