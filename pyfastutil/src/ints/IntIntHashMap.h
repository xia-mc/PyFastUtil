//
// Created by xia__mc on 2024/11/24.
//

#ifndef PYFASTUTIL_INTINTHASHMAP_H
#define PYFASTUTIL_INTINTHASHMAP_H

#include "utils/PythonPCH.h"
#include "utils/include/UnorderedDense.h"

extern "C" {
typedef struct IntIntHashMap {
    PyObject_HEAD;
    ankerl::unordered_dense::map<int, int> map;
    Py_ssize_t shape = 0;
} IntIntHashMap;
}

PyMODINIT_FUNC PyInit_IntIntHashMap();

#endif //PYFASTUTIL_INTINTHASHMAP_H
