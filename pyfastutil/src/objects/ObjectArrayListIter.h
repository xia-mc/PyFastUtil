//
// Created by xia__mc on 2024/11/17.
//

#ifndef PYFASTUTIL_OBJECTARRAYLISTITER_H
#define PYFASTUTIL_OBJECTARRAYLISTITER_H

#include "utils/PythonPCH.h"
#include "ObjectArrayList.h"

extern "C" {
typedef struct ObjectArrayListIter {
    PyObject_HEAD;
    ObjectArrayList *container;
    size_t index;
    bool reversed;
} ObjectArrayListIter;

ObjectArrayListIter *ObjectArrayListIter_create(ObjectArrayList *list, bool reversed = false);

}

PyMODINIT_FUNC PyInit_ObjectArrayListIter();

#endif //PYFASTUTIL_OBJECTARRAYLISTITER_H
