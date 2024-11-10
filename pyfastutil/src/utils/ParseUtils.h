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
static __forceinline bool PyParse_EvalRange(PyObject *args, Py_ssize_t &start, Py_ssize_t &stop, Py_ssize_t &step) {
    auto argCount = PyTuple_GET_SIZE(args);

    switch (argCount) {
        case 1:
            if (!PyArg_ParseTuple(args, "n", &stop)) {
                PyErr_SetString(PyExc_TypeError, "Args must be SupportIndex.");
                return false;
            }
            start = 0;
            step = 1;
            break;
        case 2:
            if (!PyArg_ParseTuple(args, "nn", &start, &stop)) {
                PyErr_SetString(PyExc_TypeError, "Args must be SupportIndex.");
                return false;
            }
            step = 1;
            break;
        case 3:
            if (!PyArg_ParseTuple(args, "nnn", &start, &stop, &step)) {
                PyErr_SetString(PyExc_TypeError, "Args must be SupportIndex.");
                return false;
            }

            if (step == 0) {
                PyErr_SetString(PyExc_ValueError, "Arg 3 must not be zero.");
                return false;
            }
            break;
        default:
            PyErr_SetString(PyExc_TypeError, "Expected at least 1 argument, got 0.");
            return false;
    }

    return true;
}

#endif //PYFASTUTIL_PARSEUTILS_H
