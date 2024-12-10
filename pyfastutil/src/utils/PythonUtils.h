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
static __forceinline bool PyParse_EvalRange(PyObject *&args, Py_ssize_t &start, Py_ssize_t &stop, Py_ssize_t &step) noexcept {
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
#define PyLong_DIGITS(obj) ((obj)->long_value.ob_digit)
#else
#define PyLong_DIGITS(obj) ((obj)->ob_digit)
#endif

#ifdef IS_PYTHON_312_OR_LATER
#define PyLong_TAGS(obj) ((obj)->long_value.lv_tag)
#else
#define PyLong_TAGS(obj) ((obj)->lv_tag)
#endif

static __forceinline int PyFast_AsInt(PyObject *obj) noexcept {
#ifdef IS_PYTHON_313_OR_LATER
    return PyLong_AsInt(obj);
#else
#ifdef IS_PYTHON_312_OR_LATER
    return _PyLong_AsInt(obj);
#else
    const Py_ssize_t size = Py_SIZE(obj);
    if (size == 0) {
        return 0;
    } else if (size < 0) {
        return -(int) *PyLong_DIGITS((PyLongObject *) obj);
    } else {
        return (int) *PyLong_DIGITS((PyLongObject *) obj);
    }
#endif
#endif
}

static __forceinline short PyFast_AsShort(PyObject *obj) noexcept {
    const Py_ssize_t size = Py_SIZE(obj);
    if (size == 0) {
        return 0;
    } else if (size < 0) {
        return (short) (-(int) *PyLong_DIGITS((PyLongObject *) obj));
    } else {
        return (short) *PyLong_DIGITS((PyLongObject *) obj);
    }
}

static __forceinline char PyFast_AsChar(PyObject *obj) noexcept {
    const Py_ssize_t size = Py_SIZE(obj);
    if (size == 0) {
        return 0;
    } else if (size < 0) {
        return (char) (-(int) *PyLong_DIGITS((PyLongObject *) obj));
    } else {
        return (char) *PyLong_DIGITS((PyLongObject *) obj);
    }
}

static __forceinline PyObject *PyFast_FromInt(const int value) noexcept {
#ifdef IS_PYTHON_312_OR_LATER
    return (PyObject *) _PyLong_FromDigits(value < 0, 1, (digit *) (&value));
#else
    return PyLong_FromLong(value);
#endif
}

#endif //PYFASTUTIL_PYTHONUTILS_H
