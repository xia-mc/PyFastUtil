//
// Created by xia__mc on 2024/11/9.
//

#ifndef PYFASTUTIL_PYTHONUTILS_H
#define PYFASTUTIL_PYTHONUTILS_H

#include "PythonPCH.h"
#include "Compat.h"

static __forceinline void SAFE_DECREF(PyObject *&object) {
    if (object == nullptr)
        return;
    Py_DECREF(object);
    object = nullptr;
}

template<typename T>
static __forceinline T *Py_CreateObj(PyTypeObject &typeObj) {
    return reinterpret_cast<T *>(PyObject_CallObject((PyObject *) &typeObj, nullptr));
}

template<typename T>
static __forceinline T *Py_CreateObjNoInit(PyTypeObject &typeObj) {
    return reinterpret_cast<T *>(_PyObject_New(&typeObj));
}

#endif //PYFASTUTIL_PYTHONUTILS_H
