//
// Created by xia__mc on 2024/11/9.
//

#include "PyFastUtil.h"
#include "utils/simd/BitonicSort.h"
#include "ints/IntArrayList.h"
#include "ints/IntArrayListIter.h"
#include "ints/BigIntArrayList.h"
#include "ints/BigIntArrayListIter.h"
#include "objects/ObjectArrayList.h"
#include "objects/ObjectArrayListIter.h"
#include "objects/ObjectLinkedList.h"
#include "objects/ObjectLinkedListIter.h"
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

    PyObject *parent = PyModule_Create(&pyfastutilModule);
    if (parent == nullptr)
        return nullptr;

    PyModule_AddObject(parent, "IntArrayList", PyInit_IntArrayList());
    PyModule_AddObject(parent, "IntArrayListIter", PyInit_IntArrayListIter());
    PyModule_AddObject(parent, "BigIntArrayList", PyInit_BigIntArrayList());
    PyModule_AddObject(parent, "BigIntArrayListIter", PyInit_BigIntArrayListIter());
    PyModule_AddObject(parent, "ObjectArrayList", PyInit_ObjectArrayList());
    PyModule_AddObject(parent, "ObjectArrayListIter", PyInit_ObjectArrayListIter());
    PyModule_AddObject(parent, "ObjectLinkedList", PyInit_ObjectLinkedList());
    PyModule_AddObject(parent, "ObjectLinkedListIter", PyInit_ObjectLinkedListIter());

    PyModule_AddObject(parent, "Unsafe", PyInit_Unsafe());

    return parent;
}
#pragma clang diagnostic pop
