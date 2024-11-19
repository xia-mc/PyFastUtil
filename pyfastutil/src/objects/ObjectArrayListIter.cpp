//
// Created by xia__mc on 2024/11/17.
//

#include "ObjectArrayListIter.h"
#include "utils/PythonUtils.h"

extern "C" {

static PyTypeObject ObjectArrayListIterType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
};

ObjectArrayListIter *ObjectArrayListIter_create(ObjectArrayList *list, bool reversed) {
    auto *instance = Py_CreateObjNoInit<ObjectArrayListIter>(ObjectArrayListIterType);
    if (instance == nullptr) return nullptr;

    Py_INCREF(list);
    instance->container = list;
    if (reversed) {
        instance->index = (!list->vector.empty()) ? list->vector.size() - 1 : 0;
        instance->reversed = true;
    } else {
        instance->index = 0;
        instance->reversed = false;
    }

    return instance;
}

static void ObjectArrayListIter_dealloc(ObjectArrayListIter *self) {
    SAFE_DECREF(self->container);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *ObjectArrayListIter_next(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectArrayListIter *>(pySelf);

    if (self->container->vector.empty()) {
        PyErr_SetNone(PyExc_StopIteration);
        return nullptr;
    }

    if (self->reversed) {
        if (self->index == 0) {
            // last iteration
            PyObject *element = self->container->vector[self->index];
            self->index = SIZE_MAX;
            Py_INCREF(element);
            return element;
        }
        if (self->index == SIZE_MAX) {
            // already finish iteration
            PyErr_SetNone(PyExc_StopIteration);
            return nullptr;
        }

        PyObject *element = self->container->vector[self->index];
        self->index--;
        Py_INCREF(element);
        return element;
    } else {
        if (self->index >= self->container->vector.size()) {
            PyErr_SetNone(PyExc_StopIteration);
            return nullptr;
        }

        PyObject *element = self->container->vector[self->index];
        self->index++;
        Py_INCREF(element);
        return element;
    }
}

static PyObject *ObjectArrayListIter_iter(PyObject *pySelf) {
    return pySelf;
}

static PyMethodDef ObjectArrayListIter_methods[] = {
        {nullptr}
};

static struct PyModuleDef ObjectArrayListIter_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.ObjectArrayListIter",
        "An ObjectArrayListIter_module that creates an ObjectArrayList",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

void initializeObjectArrayListIterType(PyTypeObject &type) {
    type.tp_name = "ObjectArrayListIter";
    type.tp_basicsize = sizeof(ObjectArrayListIter);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_iter = ObjectArrayListIter_iter;
    type.tp_iternext = ObjectArrayListIter_next;
    type.tp_methods = ObjectArrayListIter_methods;
    type.tp_dealloc = (destructor) ObjectArrayListIter_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_ObjectArrayListIter() {
    initializeObjectArrayListIterType(ObjectArrayListIterType);

    PyObject *object = PyModule_Create(&ObjectArrayListIter_module);
    if (object == nullptr)
        return nullptr;

    Py_INCREF(&ObjectArrayListIterType);
    if (PyModule_AddObject(object, "ObjectArrayListIter", (PyObject *) &ObjectArrayListIterType) < 0) {
        Py_DECREF(&ObjectArrayListIterType);
        Py_DECREF(object);
        return nullptr;
    }

    return object;
}
#pragma clang diagnostic pop

}