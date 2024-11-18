//
// Created by xia__mc on 2024/11/18.
//

#ifndef PYFASTUTIL_CPYTHONSORT_H
#define PYFASTUTIL_CPYTHONSORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "PythonPCH.h"


PyObject *CPython_sort(PyObject **items, Py_ssize_t size, PyObject *keyfunc, int reverse);

#ifdef __cplusplus
}
#endif

#endif //PYFASTUTIL_CPYTHONSORT_H
