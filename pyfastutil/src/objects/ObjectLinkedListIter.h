//
// Created by xia__mc on 2024/11/19.
//

#ifndef PYFASTUTIL_OBJECTLINKEDLISTITER_H
#define PYFASTUTIL_OBJECTLINKEDLISTITER_H

#include "utils/PythonPCH.h"
#include "ObjectLinkedList.h"

extern "C" {
typedef struct ObjectLinkedListIter { // NOLINT(*-pro-type-member-init)
    PyObject_HEAD;
    ObjectLinkedList *container;
    size_t index;
    std::list<PyObject *>::iterator cacheIter;
    uint64_t cacheModCount;
    bool reversed;
} ObjectLinkedListIter;

ObjectLinkedListIter *ObjectLinkedListIter_create(ObjectLinkedList *list, bool reversed = false);

}

PyMODINIT_FUNC PyInit_ObjectLinkedListIter();

#endif //PYFASTUTIL_OBJECTLINKEDLISTITER_H
