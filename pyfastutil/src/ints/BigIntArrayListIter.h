//
// Created by xia__mc on 2024/11/16.
//

#ifndef PYFASTUTIL_BIGINTARRAYLISTITER_H
#define PYFASTUTIL_BIGINTARRAYLISTITER_H

#include "utils/PythonPCH.h"
#include "BigIntArrayList.h"

extern "C" {
typedef struct BigIntArrayListIter {
    PyObject_HEAD;
    BigIntArrayList *container;
    size_t index;
    bool reversed;
} BigIntArrayListIter;

BigIntArrayListIter *BigIntArrayListIter_create(BigIntArrayList *list, bool reversed = false);

}

PyMODINIT_FUNC PyInit_BigIntArrayListIter();

#endif //PYFASTUTIL_BIGINTARRAYLISTITER_H
