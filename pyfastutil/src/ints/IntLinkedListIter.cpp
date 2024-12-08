//
// Created by xia__mc on 2024/11/24.
//

#include "IntLinkedListIter.h"
#include "utils/PythonUtils.h"
#include "utils/Utils.h"

extern "C" {

static PyTypeObject IntLinkedListIterType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
};

IntLinkedListIter *IntLinkedListIter_create(IntLinkedList *list, bool reversed) {
    auto *instance = Py_CreateObjNoInit<IntLinkedListIter>(IntLinkedListIterType);
    if (instance == nullptr) return nullptr;

    Py_INCREF(list);
    instance->container = list;
    if (reversed) {
        instance->index = (!list->list.empty()) ? list->list.size() - 1 : 0;
        instance->reversed = true;
        instance->cacheIter = --list->list.end();
    } else {
        instance->index = 0;
        instance->reversed = false;
        instance->cacheIter = list->list.begin();
    }
    instance->cacheModCount = list->modCount;

    return instance;
}

static void IntLinkedListIter_dealloc(IntLinkedListIter *self) {
    SAFE_DECREF(self->container);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static __forceinline void updateCache(IntLinkedListIter *&self) {
    if (self->cacheModCount == self->container->modCount) {
        return;
    }

    auto newIter = at(self->container->list, self->index);
    if (self->reversed) {
        self->index--;
    } else {
        self->index++;
    }
    self->cacheIter = newIter;
    self->cacheModCount = self->container->modCount;
}

static PyObject *IntLinkedListIter_next(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntLinkedListIter *>(pySelf);

    if (self->container->list.empty()) {
        PyErr_SetNone(PyExc_StopIteration);
        return nullptr;
    }

    if (self->reversed) {
        if (self->index == 0) {
            // last iteration
            int element = *(--self->container->list.end());
            self->index = SIZE_MAX;
            return PyFast_FromInt(element);
        }
        if (self->index == SIZE_MAX) {
            // already finish iteration
            PyErr_SetNone(PyExc_StopIteration);
            return nullptr;
        }

        updateCache(self);
        int element = *self->cacheIter;
        self->index--;
        self->cacheIter--;
        return PyFast_FromInt(element);
    } else {
        if (self->index >= self->container->list.size()) {
            PyErr_SetNone(PyExc_StopIteration);
            return nullptr;
        }

        updateCache(self);
        int element = *self->cacheIter;
        self->index++;
        self->cacheIter++;
        return PyFast_FromInt(element);
    }
}

static PyObject *IntLinkedListIter_iter(PyObject *pySelf) {
    Py_INCREF(pySelf);
    return pySelf;
}

static PyMethodDef IntLinkedListIter_methods[] = {
        {nullptr}
};

static struct PyModuleDef IntLinkedListIter_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.IntLinkedListIter",
        "An IntLinkedListIter_module that creates an IntLinkedList",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

void initializeIntLinkedListIterType(PyTypeObject &type) {
    type.tp_name = "IntLinkedListIter";
    type.tp_basicsize = sizeof(IntLinkedListIter);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_iter = IntLinkedListIter_iter;
    type.tp_iternext = IntLinkedListIter_next;
    type.tp_methods = IntLinkedListIter_methods;
    type.tp_dealloc = (destructor) IntLinkedListIter_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_IntLinkedListIter() {
    initializeIntLinkedListIterType(IntLinkedListIterType);

    PyObject *object = PyModule_Create(&IntLinkedListIter_module);
    if (object == nullptr)
        return nullptr;

    Py_INCREF(&IntLinkedListIterType);
    if (PyModule_AddObject(object, "IntLinkedListIter", (PyObject *) &IntLinkedListIterType) < 0) {
        Py_DECREF(&IntLinkedListIterType);
        Py_DECREF(object);
        return nullptr;
    }

    return object;
}
#pragma clang diagnostic pop

}
