//
// Created by xia__mc on 2024/11/16.
//

#ifndef PYFASTUTIL_BIGINTARRAYLIST_H
#define PYFASTUTIL_BIGINTARRAYLIST_H

#include "utils/PythonPCH.h"
#include "utils/memory/AlignedAllocator.h"
#include <vector>

extern "C" {
typedef struct BigIntArrayList {
    PyObject_HEAD;
    // we use 64 bytes memory aligned to support faster SIMD, suggestion by ChatGPT.
    std::vector<long long, AlignedAllocator<long long, 64>> vector;
    Py_ssize_t shape = 0;
} BigIntArrayList;
}

PyMODINIT_FUNC PyInit_BigIntArrayList();

#endif //PYFASTUTIL_BIGINTARRAYLIST_H
