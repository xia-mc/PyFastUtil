//
// Created by xia__mc on 2024/11/24.
//

#ifndef PYFASTUTIL_INTLINKEDLISTITER_H
#define PYFASTUTIL_INTLINKEDLISTITER_H

#include "utils/PythonPCH.h"
#include "ints/IntLinkedList.h"

extern "C" {
typedef struct IntLinkedListIter { // NOLINT(*-pro-type-member-init)
    PyObject_HEAD;
    IntLinkedList *container;
    size_t index;
    std::list<int>::iterator cacheIter;
    uint64_t cacheModCount;
    bool reversed;
} IntLinkedListIter;

IntLinkedListIter *IntLinkedListIter_create(IntLinkedList *list, bool reversed = false);

}

PyMODINIT_FUNC PyInit_IntLinkedListIter();

#endif //PYFASTUTIL_INTLINKEDLISTITER_H
