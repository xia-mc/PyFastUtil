//
// Created by xia__mc on 2024/12/10.
//

#ifndef PYFASTUTIL_ASM_H
#define PYFASTUTIL_ASM_H

#include "utils/PythonPCH.h"

extern "C" {
typedef struct ASM {
    PyObject_HEAD;
} ASM;
}

PyMODINIT_FUNC PyInit_ASM();

#endif //PYFASTUTIL_ASM_H
