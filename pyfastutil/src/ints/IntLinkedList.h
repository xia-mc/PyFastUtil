//
// Created by xia__mc on 2024/11/24.
//

#ifndef PYFASTUTIL_INTLINKEDLIST_H
#define PYFASTUTIL_INTLINKEDLIST_H

#include "utils/PythonPCH.h"
#include <list>

extern "C" {
typedef struct IntLinkedList {
    PyObject_HEAD;
    std::list<int> list;
    uint64_t modCount;
} IntLinkedList;
}

PyMODINIT_FUNC PyInit_IntLinkedList();

#endif //PYFASTUTIL_INTLINKEDLIST_H
