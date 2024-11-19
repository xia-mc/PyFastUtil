//
// Created by xia__mc on 2024/11/17.
//

#ifndef PYFASTUTIL_OBJECTARRAYLIST_H
#define PYFASTUTIL_OBJECTARRAYLIST_H

#include "utils/PythonPCH.h"
#include <vector>

extern "C" {
typedef struct ObjectArrayList {
    PyObject_HEAD;
    std::vector<PyObject *> vector;
} ObjectArrayList;
}

PyMODINIT_FUNC PyInit_ObjectArrayList();

#endif //PYFASTUTIL_OBJECTARRAYLIST_H
