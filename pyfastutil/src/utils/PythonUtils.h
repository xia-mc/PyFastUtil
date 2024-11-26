//
// Created by xia__mc on 2024/11/9.
//

#ifndef PYFASTUTIL_PYTHONUTILS_H
#define PYFASTUTIL_PYTHONUTILS_H

#include <iostream>
#include "PythonPCH.h"
#include "Compat.h"

template<typename T>
static __forceinline void SAFE_DECREF(T *&object) noexcept {
    assert(object != nullptr);
    Py_DECREF(object);
    object = nullptr;
}

template<typename T>
static __forceinline T *Py_CreateObj(PyTypeObject &typeObj) noexcept {
    return (T *) PyObject_CallObject((PyObject *) &typeObj, nullptr);
}

template<typename T>
static __forceinline T *Py_CreateObj(PyTypeObject &typeObj, PyObject *args) noexcept {
    return (T *) PyObject_CallObject((PyObject *) &typeObj, args);
}

template<typename T>
static __forceinline T *Py_CreateObjNoInit(PyTypeObject &typeObj) noexcept {
    return (T *) PyObject_New(T, &typeObj);
}

/**
 * Try to parse args like built-in range function
 * If not successful, function will raise python exception.
 * @return if successful
 */
static inline bool PyParse_EvalRange(PyObject *&args, Py_ssize_t &start, Py_ssize_t &stop, Py_ssize_t &step) noexcept {
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

#ifdef IS_PYTHON_312_OR_LATER
#define PyLong_DIGITS(obj) (obj->long_value.ob_digit)
#else
#define PyLong_DIGITS(obj) (obj->ob_digit)
#endif

static __forceinline int PyFast_AsInt(PyObject *obj) noexcept {
    const auto *longObj = (PyLongObject *) obj;

    const Py_ssize_t size = Py_SIZE(longObj);
    if (size == 0) {
        return 0;
    }

    const int result = (int) PyLong_DIGITS(longObj)[0];
    return size < 0 ? -result : result;
}

static __forceinline long long PyFast_AsLongLong(PyObject *obj) noexcept {
    const auto *longObj = (PyLongObject *) obj;

    const Py_ssize_t size = Py_SIZE(longObj);
    if (size == 0) {
        return 0;
    }

    const auto absSize = Py_ABS(size);

    unsigned long long value = 0;
    for (Py_ssize_t i = 0; i < absSize; i++) {
        value += (unsigned long long) PyLong_DIGITS(longObj)[i] << (i * PyLong_SHIFT);
    }

    return size < 0 ? -(long long) value : (long long) value;
}

#endif //PYFASTUTIL_PYTHONUTILS_H
