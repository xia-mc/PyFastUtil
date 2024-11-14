//
// Created by xia__mc on 2024/11/7.
//

#ifndef PYFASTUTIL_INTARRAYLIST_H
#define PYFASTUTIL_INTARRAYLIST_H

#include "utils/PythonPCH.h"
#include "utils/memory/AlignedAllocator.h"
#include <vector>

extern "C" {
typedef struct IntArrayList {
    PyObject_HEAD;
    // we use 64 bytes memory aligned to support faster SIMD, suggestion by ChatGPT.
    std::vector<int, AlignedAllocator<int, 64>> vector;
} IntArrayList;
}

PyMODINIT_FUNC PyInit_IntArrayList();

#endif //PYFASTUTIL_INTARRAYLIST_H
