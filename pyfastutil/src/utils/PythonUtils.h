//
// Created by xia__mc on 2024/11/9.
//

#ifndef PYFASTUTIL_PYTHONUTILS_H
#define PYFASTUTIL_PYTHONUTILS_H

#include "PythonPCH.h"
#include "Compat.h"

template<typename T>
static __forceinline void SAFE_DECREF(T *&object) {
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
static __forceinline T *Py_CreateObj(PyTypeObject &typeObj, PyObject *args) {
    return reinterpret_cast<T *>(PyObject_CallObject((PyObject *) &typeObj, args));
}

template<typename T>
static __forceinline T *Py_CreateObjNoInit(PyTypeObject &typeObj) {
    return reinterpret_cast<T *>(PyObject_New(T, &typeObj));
}

/**
 * Try to parse args like built-in range function
 * If not successful, function will raise python exception.
 * @return if successful
 */
static inline bool PyParse_EvalRange(PyObject *&args, Py_ssize_t &start, Py_ssize_t &stop, Py_ssize_t &step) {
    Py_ssize_t arg1 = PY_SSIZE_T_MAX;
    Py_ssize_t arg2 = PY_SSIZE_T_MAX;
    Py_ssize_t arg3 = PY_SSIZE_T_MAX;

    if (!PyArg_ParseTuple(args, "n|nn", &arg1, &arg2, &arg3)) {
        return false;
    }

    if (arg2 == PY_SSIZE_T_MAX) {
        start = 0;
        stop = arg1;
        step = 1;
    } else if (arg3 == PY_SSIZE_T_MAX) {
        start = arg1;
        stop = arg2;
        step = 1;
    } else if (arg3 != 0) {
        start = arg1;
        stop = arg2;
        step = arg3;
    } else {
        PyErr_SetString(PyExc_ValueError, "Arg 3 must not be zero.");
        return false;
    }

    return true;
}

#define Py_RETURN_BOOL(b) { if (b) Py_RETURN_TRUE; else Py_RETURN_FALSE; }

#endif //PYFASTUTIL_PYTHONUTILS_H
