//
// Created by xia__mc on 2024/11/9.
//

#include "PyFastUtil.h"
#include "utils/simd/BitonicSort.h"
#include "ints/IntArrayList.h"
#include "unsafe/Unsafe.h"

static struct PyModuleDef pyfastutilModule = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil",
        "C++ implementation of PyFastUtil.",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit___pyfastutil() {
    simd::init();

    PyObject* parent = PyModule_Create(&pyfastutilModule);
    if (parent == nullptr)
        return nullptr;

    PyModule_AddObject(parent, "IntArrayList", PyInit_IntArrayList());
    PyModule_AddObject(parent, "Unsafe", PyInit_Unsafe());

    return parent;
}
#pragma clang diagnostic pop
