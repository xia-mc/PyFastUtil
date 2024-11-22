//
// Created by xia__mc on 2024/11/19.
//

#include "ObjectLinkedListIter.h"
#include "utils/PythonUtils.h"
#include "utils/Utils.h"

extern "C" {

static PyTypeObject ObjectLinkedListIterType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
};

ObjectLinkedListIter *ObjectLinkedListIter_create(ObjectLinkedList *list, bool reversed) {
    auto *instance = Py_CreateObjNoInit<ObjectLinkedListIter>(ObjectLinkedListIterType);
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

static void ObjectLinkedListIter_dealloc(ObjectLinkedListIter *self) {
    SAFE_DECREF(self->container);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static __forceinline void updateCache(ObjectLinkedListIter *&self) {
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

static PyObject *ObjectLinkedListIter_next(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectLinkedListIter *>(pySelf);

    if (self->container->list.empty()) {
        PyErr_SetNone(PyExc_StopIteration);
        return nullptr;
    }

    if (self->reversed) {
        if (self->index == 0) {
            // last iteration
            PyObject *element = *(--self->container->list.end());
            self->index = SIZE_MAX;
            Py_INCREF(element);
            return element;
        }
        if (self->index == SIZE_MAX) {
            // already finish iteration
            PyErr_SetNone(PyExc_StopIteration);
            return nullptr;
        }

        updateCache(self);
        PyObject *element = *self->cacheIter;
        self->index--;
        self->cacheIter--;
        Py_INCREF(element);
        return element;
    } else {
        if (self->index >= self->container->list.size()) {
            PyErr_SetNone(PyExc_StopIteration);
            return nullptr;
        }

        updateCache(self);
        PyObject *element = *self->cacheIter;
        self->index++;
        self->cacheIter++;
        Py_INCREF(element);
        return element;
    }
}

static PyObject *ObjectLinkedListIter_iter(PyObject *pySelf) {
    Py_INCREF(pySelf);
    return pySelf;
}

static PyMethodDef ObjectLinkedListIter_methods[] = {
        {nullptr}
};

static struct PyModuleDef ObjectLinkedListIter_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.ObjectLinkedListIter",
        "An ObjectLinkedListIter_module that creates an ObjectLinkedList",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

void initializeObjectLinkedListIterType(PyTypeObject &type) {
    type.tp_name = "ObjectLinkedListIter";
    type.tp_basicsize = sizeof(ObjectLinkedListIter);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_iter = ObjectLinkedListIter_iter;
    type.tp_iternext = ObjectLinkedListIter_next;
    type.tp_methods = ObjectLinkedListIter_methods;
    type.tp_dealloc = (destructor) ObjectLinkedListIter_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_ObjectLinkedListIter() {
    initializeObjectLinkedListIterType(ObjectLinkedListIterType);

    PyObject *object = PyModule_Create(&ObjectLinkedListIter_module);
    if (object == nullptr)
        return nullptr;

    Py_INCREF(&ObjectLinkedListIterType);
    if (PyModule_AddObject(object, "ObjectLinkedListIter", (PyObject *) &ObjectLinkedListIterType) < 0) {
        Py_DECREF(&ObjectLinkedListIterType);
        Py_DECREF(object);
        return nullptr;
    }

    return object;
}
#pragma clang diagnostic pop

}