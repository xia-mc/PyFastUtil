//
// Created by xia__mc on 2024/11/7.
//

#include "IntArrayList.h"
#include <vector>
#include "algorithm"
#include "utils/PointerUtils.h"
#include "utils/ParseUtils.h"

extern "C" {
typedef struct IntArrayList {
    PyObject_HEAD;
    std::vector<long> vector = std::vector<long>();
} IntArrayList;

static PyTypeObject IntArrayListType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
};

__forceinline void parseArgs(PyObject *args, PyObject *kwargs, PyObject *&pyIterable, Py_ssize_t &pySize) {
    static constexpr const char *kwlist[] = {"__iterable", "pySize", nullptr};

    PyObject *arg1 = nullptr;

    // parse args
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|On", const_cast<char **>(kwlist), &arg1, &pySize)) {
        return;
    }

    if (arg1 == nullptr) return;

    if (PyLong_Check(arg1)) {
        pySize = PyLong_AsSsize_t(arg1);
    } else {
        pyIterable = arg1;
    }
}

static int IntArrayList_init(IntArrayList *self, PyObject *args, PyObject *kwargs) {
    PyObject *pyIterable = nullptr;
    Py_ssize_t pySize = -1;

    parseArgs(args, kwargs, pyIterable, pySize);

    // init vector
    try {
        if (pySize > 0) {
            self->vector.reserve(static_cast<size_t>(pySize));
        }

        if (pyIterable != nullptr) {
            PyObject *iter = PyObject_GetIter(pyIterable);
            if (iter == nullptr) {
                PyErr_SetString(PyExc_TypeError, "Arg '__iterable' is not iterable.");
                return -1;
            }

            PyObject *item;
            while ((item = PyIter_Next(iter)) != nullptr) {
                long value = PyLong_AsLong(item);
                if (PyErr_Occurred()) {
                    SAFE_DECREF(iter);
                    SAFE_DECREF(item);
                    PyErr_SetString(PyExc_RuntimeError, "Failed to convert item to C long during iteration.");
                    return -1;
                }
                self->vector.push_back(value);
                SAFE_DECREF(item);
            }
            SAFE_DECREF(iter);
            if (PyErr_Occurred()) return -1;
        }
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }

    return 0;
}

static void IntArrayList_dealloc(IntArrayList *self) {
    self->vector.~vector();
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *IntArrayList_from_range(PyObject *args) {
    Py_ssize_t start, stop, step;

    if (!PyParse_EvalRange(args, start, stop, step)) {
        return nullptr;
    }

    IntArrayList *list = PyObject_New(IntArrayList, &IntArrayListType);
    if (list == nullptr) return PyErr_NoMemory();

    try {
        // PyParse_EvalRange ensure step != 0
        if ((step > 0 && start >= stop) || (step < 0 && start <= stop)) {
            return reinterpret_cast<PyObject *>(list);
        }

        if (step > 0) {
            for (auto i = start; i < stop; i += step) {
                list->vector.push_back(static_cast<long>(i));
            }
        } else {
            for (auto i = start; i > stop; i += step) {
                list->vector.push_back(static_cast<long>(i));
            }
        }
    } catch (const std::exception &e) {
        PyObject_Del(list);
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    return reinterpret_cast<PyObject *>(list);
}

static PyObject *IntArrayList_copy(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    IntArrayList *copy = PyObject_New(IntArrayList, &IntArrayListType);
    if (copy == nullptr) return PyErr_NoMemory();

    try {
        copy->vector = self->vector;
    } catch (const std::exception &e) {
        PyObject_Del(copy);
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    return reinterpret_cast<PyObject *>(copy);
}

static PyObject *IntArrayList_append(PyObject *pySelf, PyObject *object) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    long value = PyLong_AsLong(object);
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_TypeError, "Arg '__object' must be an integer.");
        return nullptr;
    }

    try {
        self->vector.push_back(value);
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    Py_RETURN_NONE;
}

static PyObject *IntArrayList_extend(PyObject *pySelf, PyObject *iterable) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    if (iterable != nullptr) {
        // int list
        if (Py_TYPE(iterable) == &IntArrayListType) {  // IntArrayList is a final class
            auto *iter = reinterpret_cast<IntArrayList *>(iterable);
            self->vector.insert(self->vector.end(), iter->vector.begin(), iter->vector.end());
            Py_RETURN_NONE;
        }

        // python iterable
        PyObject *iter = PyObject_GetIter(iterable);
        if (iter == nullptr) {
            return nullptr;
        }

        Py_ssize_t hint = PyObject_LengthHint(iterable, 0);
        if (hint > 0) {
            self->vector.reserve(self->vector.size() + hint);
        }

        PyObject *item;
        try {
            while ((item = PyIter_Next(iter)) != nullptr) {
                long value = PyLong_AsLong(item);
                SAFE_DECREF(item);

                if (PyErr_Occurred()) {
                    SAFE_DECREF(iter);
                    return nullptr;
                }

                self->vector.push_back(value);
            }
        } catch (const std::exception &e) {
            PyErr_SetString(PyExc_RuntimeError, e.what());
            SAFE_DECREF(iter);
            return nullptr;
        }

        SAFE_DECREF(iter);
    }

    Py_RETURN_NONE;
}

static PyObject *IntArrayList_pop(PyObject *pySelf, PyObject *args) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    const auto vecSize = static_cast<Py_ssize_t>(self->vector.size());
    Py_ssize_t pyIndex = vecSize - 1;

    if (!PyArg_ParseTuple(args, "|n", &pyIndex)) {
        PyErr_SetString(PyExc_TypeError, "pyIndex must be an integer.");
        return nullptr;
    }

    if (pyIndex < 0) {
        pyIndex = vecSize + pyIndex;
    }

    if (pyIndex < 0 || pyIndex >= vecSize) {
        PyErr_SetString(PyExc_IndexError, "pyIndex out of range.");
        return nullptr;
    }


    try {
        // cache the return value
        const auto popped = self->vector[static_cast<size_t>(pyIndex)];
        // do pop
        self->vector.erase(self->vector.begin() + pyIndex);

        PyObject *pyPopped = PyLong_FromLong(popped);
        if (!pyPopped) {
            return nullptr;
        }
        return pyPopped;
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static PyObject *IntArrayList_index(PyObject *pySelf, PyObject *args) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    long value;
    Py_ssize_t start = 0;
    auto stop = static_cast<Py_ssize_t>(self->vector.size());

    if (!PyArg_ParseTuple(args, "l|nn", &value, &start, &stop)) {
        PyErr_SetString(PyExc_TypeError, "Args must be (value, [start, [stop]]).");
        return nullptr;
    }

    // valid check
    if (start < 0 || start > static_cast<Py_ssize_t>(self->vector.size())) {
        PyErr_SetString(PyExc_IndexError, "start index out of range.");
        return nullptr;
    }
    if (stop < 0 || stop > static_cast<Py_ssize_t>(self->vector.size())) {
        PyErr_SetString(PyExc_IndexError, "stop index out of range.");
        return nullptr;
    }

    // do index
    auto it = std::find(self->vector.begin() + start, self->vector.begin() + stop, value);

    if (it == self->vector.end()) {
        PyErr_SetString(PyExc_ValueError, "Value is not in list.");
        return nullptr;
    }

    Py_ssize_t index = std::distance(self->vector.begin(), it);
    return PyLong_FromSsize_t(index);
}


static PyMethodDef IntArrayList_methods[] = {
        {"from_range", (PyCFunction) IntArrayList_from_range, METH_VARARGS | METH_STATIC},
        {"copy", (PyCFunction) IntArrayList_copy,             METH_NOARGS},
        {"append", (PyCFunction) IntArrayList_append,         METH_O},
        {"extend", (PyCFunction) IntArrayList_extend,         METH_O},
        {"pop", (PyCFunction) IntArrayList_pop,               METH_VARARGS},
        {"index", (PyCFunction) IntArrayList_index,           METH_VARARGS},
        {nullptr}
};

static struct PyModuleDef IntArrayList_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.IntArrayList",
        "A IntArrayList_module that creates an IntArrayList",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

void initializeIntArrayListType(PyTypeObject &type) {
    type.tp_name = "IntArrayList";
    type.tp_basicsize = sizeof(IntArrayList);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_methods = IntArrayList_methods;
    type.tp_init = (initproc) IntArrayList_init;
    type.tp_new = PyType_GenericNew;
    type.tp_dealloc = (destructor) IntArrayList_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_IntArrayList() {
    initializeIntArrayListType(IntArrayListType);

    PyObject *object = PyModule_Create(&IntArrayList_module);
    if (object == nullptr)
        return nullptr;

    Py_INCREF(&IntArrayListType);
    if (PyModule_AddObject(object, "IntArrayList", (PyObject *) &IntArrayListType) < 0) {
        Py_DECREF(&IntArrayListType);
        Py_DECREF(object);
        return nullptr;
    }

    return object;
}
#pragma clang diagnostic pop

}
