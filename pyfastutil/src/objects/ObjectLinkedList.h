//
// Created by xia__mc on 2024/11/19.
//

#ifndef PYFASTUTIL_OBJECTLINKEDLIST_H
#define PYFASTUTIL_OBJECTLINKEDLIST_H

#include "utils/PythonPCH.h"
#include <list>

extern "C" {
typedef struct ObjectLinkedList {
    PyObject_HEAD;
    std::list<PyObject *> list;
    uint64_t modCount;
} ObjectLinkedList;
}

PyMODINIT_FUNC PyInit_ObjectLinkedList();

#endif //PYFASTUTIL_OBJECTLINKEDLIST_H
