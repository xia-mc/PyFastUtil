//
// Created by xia__mc on 2024/11/10.
//

#ifndef PYFASTUTIL_PARSEUTILS_H
#define PYFASTUTIL_PARSEUTILS_H

#include "PythonPCH.h"

/**
 * Try to parse args like built-in range function
 * If not successful, function will raise python exception.
 * @return if successful
 */
static __forceinline bool PyParse_EvalRange(PyObject *&args, Py_ssize_t &start, Py_ssize_t &stop, Py_ssize_t &step) {
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

#endif //PYFASTUTIL_PARSEUTILS_H
