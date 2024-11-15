//
// Created by xia__mc on 2024/11/13.
//

#include "IntArrayListIter.h"
#include "utils/PythonUtils.h"

extern "C" {

static PyTypeObject IntArrayListIterType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
};

IntArrayListIter *IntArrayListIter_create(IntArrayList *list, bool reversed) {
    auto *instance = Py_CreateObjNoInit<IntArrayListIter>(IntArrayListIterType);
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

static void IntArrayListIter_dealloc(IntArrayListIter *self) {
    SAFE_DECREF(self->container);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *IntArrayListIter_next(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntArrayListIter *>(pySelf);

    if (self->reversed) {
        if (self->index == 0) {
            // last iteration
            int element = self->container->vector[self->index];
            self->index = SIZE_MAX;
            return PyLong_FromLong(static_cast<long>(element));
        }
        if (self->index == SIZE_MAX) {
            // already finish iteration
            PyErr_SetNone(PyExc_StopIteration);
            return nullptr;
        }

        int element = self->container->vector[self->index];
        self->index--;
        return PyLong_FromLong(static_cast<long>(element));
    } else {
        if (self->index >= self->container->vector.size()) {
            PyErr_SetNone(PyExc_StopIteration);
            return nullptr;
        }

        int element = self->container->vector[self->index];
        self->index++;
        return PyLong_FromLong(static_cast<long>(element));
    }
}

static PyObject *IntArrayListIter_iter(PyObject *pySelf) {
    return pySelf;
}

static PyMethodDef IntArrayListIter_methods[] = {
        {nullptr}
};

static struct PyModuleDef IntArrayListIter_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.IntArrayListIter",
        "An IntArrayListIter_module that creates an IntArrayList",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

void initializeIntArrayListIterType(PyTypeObject &type) {
    type.tp_name = "IntArrayListIter";
    type.tp_basicsize = sizeof(IntArrayListIter);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_iter = IntArrayListIter_iter;
    type.tp_iternext = IntArrayListIter_next;
    type.tp_methods = IntArrayListIter_methods;
    type.tp_dealloc = (destructor) IntArrayListIter_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_IntArrayListIter() {
    initializeIntArrayListIterType(IntArrayListIterType);

    PyObject *object = PyModule_Create(&IntArrayListIter_module);
    if (object == nullptr)
        return nullptr;

    Py_INCREF(&IntArrayListIterType);
    if (PyModule_AddObject(object, "IntArrayListIter", (PyObject *) &IntArrayListIterType) < 0) {
        Py_DECREF(&IntArrayListIterType);
        Py_DECREF(object);
        return nullptr;
    }

    return object;
}
#pragma clang diagnostic pop

}
