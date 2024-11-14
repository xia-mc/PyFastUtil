//
// Created by xia__mc on 2024/11/12.
//

#ifndef PYFASTUTIL_UNSAFE_H
#define PYFASTUTIL_UNSAFE_H

#include "utils/PythonPCH.h"

extern "C" {
typedef struct Unsafe {
    PyObject_HEAD;
} Unsafe;
}

PyMODINIT_FUNC PyInit_Unsafe();

#endif //PYFASTUTIL_UNSAFE_H
