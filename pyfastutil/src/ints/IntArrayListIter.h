//
// Created by xia__mc on 2024/11/13.
//

#ifndef PYFASTUTIL_INTARRAYLISTITER_H
#define PYFASTUTIL_INTARRAYLISTITER_H

#include "utils/PythonPCH.h"
#include "IntArrayList.h"

extern "C" {
typedef struct IntArrayListIter {
    PyObject_HEAD;
    IntArrayList *container;
    size_t index;
    bool reversed;
} IntArrayListIter;

IntArrayListIter *IntArrayListIter_create(IntArrayList *list, bool reversed = false);

}

PyMODINIT_FUNC PyInit_IntArrayListIter();

#endif //PYFASTUTIL_INTARRAYLISTITER_H
