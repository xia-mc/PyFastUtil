//
// Created by xia__mc on 2024/11/16.
//

#include "BigIntArrayListIter.h"
#include "utils/PythonUtils.h"

extern "C" {

static PyTypeObject BigIntArrayListIterType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
};

BigIntArrayListIter *BigIntArrayListIter_create(BigIntArrayList *list, bool reversed) {
    auto *instance = Py_CreateObjNoInit<BigIntArrayListIter>(BigIntArrayListIterType);
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

static void BigIntArrayListIter_dealloc(BigIntArrayListIter *self) {
    SAFE_DECREF(self->container);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *BigIntArrayListIter_next(PyObject *pySelf) {
    auto *self = reinterpret_cast<BigIntArrayListIter *>(pySelf);

    if (self->container->vector.empty()) {
        PyErr_SetNone(PyExc_StopIteration);
        return nullptr;
    }

    if (self->reversed) {
        if (self->index == 0) {
            // last iteration
            long long element = self->container->vector[self->index];
            self->index = SIZE_MAX;
            return PyFast_FromLongLong(element);
        }
        if (self->index == SIZE_MAX) {
            // already finish iteration
            PyErr_SetNone(PyExc_StopIteration);
            return nullptr;
        }

        long long element = self->container->vector[self->index];
        self->index--;
        return PyFast_FromLongLong(element);
    } else {
        if (self->index >= self->container->vector.size()) {
            PyErr_SetNone(PyExc_StopIteration);
            return nullptr;
        }

        long long element = self->container->vector[self->index];
        self->index++;
        return PyFast_FromLongLong(element);
    }
}

static PyObject *BigIntArrayListIter_iter(PyObject *pySelf) {
    Py_INCREF(pySelf);
    return pySelf;
}

static PyMethodDef BigIntArrayListIter_methods[] = {
        {nullptr}
};

static struct PyModuleDef BigIntArrayListIter_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.BigIntArrayListIter",
        "An BigIntArrayListIter_module that creates an BigIntArrayList",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

void initializeBigIntArrayListIterType(PyTypeObject &type) {
    type.tp_name = "BigIntArrayListIter";
    type.tp_basicsize = sizeof(BigIntArrayListIter);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_iter = BigIntArrayListIter_iter;
    type.tp_iternext = BigIntArrayListIter_next;
    type.tp_methods = BigIntArrayListIter_methods;
    type.tp_dealloc = (destructor) BigIntArrayListIter_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_BigIntArrayListIter() {
    initializeBigIntArrayListIterType(BigIntArrayListIterType);

    PyObject *object = PyModule_Create(&BigIntArrayListIter_module);
    if (object == nullptr)
        return nullptr;

    Py_INCREF(&BigIntArrayListIterType);
    if (PyModule_AddObject(object, "BigIntArrayListIter", (PyObject *) &BigIntArrayListIterType) < 0) {
        Py_DECREF(&BigIntArrayListIterType);
        Py_DECREF(object);
        return nullptr;
    }

    return object;
}
#pragma clang diagnostic pop

}
