//
// Created by xia__mc on 2024/11/9.
//

#ifndef PYFASTUTIL_POINTERUTILS_H
#define PYFASTUTIL_POINTERUTILS_H

#include "PythonPCH.h"

static __forceinline void SAFE_DECREF(PyObject *&object) {
    if (object == nullptr)
        return;
    Py_DECREF(object);
    object = nullptr;
}

#endif //PYFASTUTIL_POINTERUTILS_H
