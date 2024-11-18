//
// Created by xia__mc on 2024/11/12.
//

#include "Unsafe.h"
#include "mutex"

extern "C" {

static PyTypeObject UnsafeType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
};

static int Unsafe_init([[maybe_unused]] Unsafe *self,
                       [[maybe_unused]] PyObject *args, [[maybe_unused]] PyObject *kwargs) {
    return 0;
}

static void Unsafe_dealloc(Unsafe *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *Unsafe_enter(PyObject *self, [[maybe_unused]] PyObject *args) {
    Py_INCREF(self);
    return self;
}

static PyObject *Unsafe_exit([[maybe_unused]] PyObject *pySelf, [[maybe_unused]] PyObject *args) {
    Py_RETURN_NONE;
}

static PyObject *Unsafe_malloc([[maybe_unused]] PyObject *self, PyObject *pySize) {
    const size_t size = PyLong_AsSize_t(pySize);
    if (PyErr_Occurred()) {
        return nullptr;
    }

    void *ptr = malloc(size);
    if (ptr == nullptr) {
        PyErr_SetString(PyExc_MemoryError, "Failed to alloc memory.");
        return nullptr;
    }
    return PyLong_FromSize_t(reinterpret_cast<uintptr_t>(ptr));
}

static PyObject *Unsafe_calloc([[maybe_unused]] PyObject *self, PyObject *args) {
    size_t num, size;
    if (!PyArg_ParseTuple(args, "KK", &num, &size)) {
        return nullptr;
    }

    void *ptr = calloc(num, size);
    if (ptr == nullptr) {
        PyErr_SetString(PyExc_MemoryError, "Failed to alloc memory.");
        return nullptr;
    }
    return PyLong_FromSize_t(reinterpret_cast<uintptr_t>(ptr));
}

static PyObject *Unsafe_realloc([[maybe_unused]] PyObject *self, PyObject *args) {
    uintptr_t address;
    size_t newSize;
    if (!PyArg_ParseTuple(args, "KK", &address, &newSize)) {
        return nullptr;
    }

    void *ptr = realloc(reinterpret_cast<void *>(address), newSize);
    if (ptr == nullptr) {
        PyErr_SetString(PyExc_MemoryError, "Failed to realloc memory.");
        return nullptr;
    }
    return PyLong_FromSize_t(reinterpret_cast<uintptr_t>(ptr));
}

static PyObject *Unsafe_free([[maybe_unused]] PyObject *self, PyObject *pyAddress) {
    const uintptr_t address = PyLong_AsSize_t(pyAddress);
    if (PyErr_Occurred()) {
        return nullptr;
    }

    free(reinterpret_cast<void *>(address));
    Py_RETURN_NONE;
}

static PyObject *Unsafe_get([[maybe_unused]] PyObject *self, PyObject *args) {
    uintptr_t address;
    Py_ssize_t size;
    if (!PyArg_ParseTuple(args, "KK", &address, &size)) {
        return nullptr;
    }

    char *ptr = reinterpret_cast<char *>(address);
    return PyBytes_FromStringAndSize(ptr, size);
}

static PyObject *Unsafe_set([[maybe_unused]] PyObject *self, PyObject *args) {
    size_t address;
    PyObject *pyBytes;

    if (!PyArg_ParseTuple(args, "KO", &address, &pyBytes)) {
        return nullptr;
    }

    if (!PyBytes_Check(pyBytes)) {
        PyErr_SetString(PyExc_TypeError, "Expected a bytes object.");
        return nullptr;
    }

    const char *bytes = PyBytes_AsString(pyBytes);
    if (bytes == nullptr) {
        PyErr_SetString(PyExc_TypeError, "Invalid bytes.");
        return nullptr;
    }

    const Py_ssize_t pySize = PyBytes_Size(pyBytes);
    if (pySize == -1 || pySize > SIZE_MAX) {
        PyErr_SetString(PyExc_TypeError, "Invalid bytes.");
        return nullptr;
    }
    const auto size = static_cast<size_t>(pySize);

    char *ptr = reinterpret_cast<char *>(address);
    memcpy(ptr, bytes, size);

    Py_RETURN_NONE;
}

static PyObject *Unsafe_getAddress([[maybe_unused]] PyObject *self, PyObject *pyObject) {
    return PyLong_FromSize_t(reinterpret_cast<uintptr_t>(pyObject));
}

static PyObject *Unsafe_as_object([[maybe_unused]] PyObject *self, PyObject *pyObject) {
    return reinterpret_cast<PyObject *>(PyLong_AsSize_t(pyObject));
}

static PyObject *Unsafe_memcpy([[maybe_unused]] PyObject *self, PyObject *args) {
    uintptr_t addressFrom;
    uintptr_t addressTo;
    size_t size;
    if (!PyArg_ParseTuple(args, "KKK", &addressFrom, &addressTo, &size)) {
        return nullptr;
    }

    memcpy(reinterpret_cast<void *>(addressTo), reinterpret_cast<void *>(addressFrom), size);

    Py_RETURN_NONE;
}

static PyObject *Unsafe_incref([[maybe_unused]] PyObject *self, PyObject *pyObject) {
    Py_INCREF(pyObject);
    Py_RETURN_NONE;
}

static PyObject *Unsafe_decref([[maybe_unused]] PyObject *self, PyObject *pyObject) {
    Py_DECREF(pyObject);
    Py_RETURN_NONE;
}

static PyObject *Unsafe_refcnt([[maybe_unused]] PyObject *self, PyObject *pyObject) {
    return PyLong_FromSsize_t(Py_REFCNT(pyObject));
}

static PyObject *Unsafe_fputs([[maybe_unused]] PyObject *pySelf, PyObject *pyStr) {
    if (!PyUnicode_Check(pyStr)) {
        PyErr_SetString(PyExc_TypeError, "Argument must be a string.");
        return nullptr;
    }

    const char *str = PyUnicode_AsUTF8(pyStr);
    if (str == nullptr) {
        return nullptr;
    }

    fputs(str, stdout);

    Py_RETURN_NONE;
}

static PyObject *Unsafe_fflush([[maybe_unused]] PyObject *pySelf) {
    fflush(stdout);

    Py_RETURN_NONE;
}

static PyObject *Unsafe_fgets([[maybe_unused]] PyObject *pySelf, PyObject *pyBufferSize) {
    if (!PyLong_Check(pyBufferSize)) {
        PyErr_SetString(PyExc_TypeError, "Argument must be an integer.");
        return nullptr;
    }

    int bufferSize = (int) PyLong_AsLong(pyBufferSize);
    if (bufferSize <= 0) {
        PyErr_SetString(PyExc_ValueError, "Buffer size must be positive.");
        return nullptr;
    }

    char *buffer = new char[bufferSize];

    if (fgets(buffer, bufferSize, stdin) == nullptr) {
        PyErr_SetString(PyExc_RuntimeError, "Error reading from stdin.");
        return nullptr;
    }

    return Py_BuildValue("s", buffer);
}

static PyMethodDef Unsafe_methods[] = {
        {"__enter__",   (PyCFunction) Unsafe_enter,      METH_NOARGS,  nullptr},
        {"__exit__",    (PyCFunction) Unsafe_exit,       METH_VARARGS, nullptr},
        {"malloc",      (PyCFunction) Unsafe_malloc,     METH_O,       nullptr},
        {"calloc",      (PyCFunction) Unsafe_calloc,     METH_VARARGS, nullptr},
        {"realloc",     (PyCFunction) Unsafe_realloc,    METH_VARARGS, nullptr},
        {"free",        (PyCFunction) Unsafe_free,       METH_O,       nullptr},
        {"get",         (PyCFunction) Unsafe_get,        METH_VARARGS, nullptr},
        {"set",         (PyCFunction) Unsafe_set,        METH_VARARGS, nullptr},
        {"get_address", (PyCFunction) Unsafe_getAddress, METH_O,       nullptr},
        {"as_object",   (PyCFunction) Unsafe_as_object,  METH_O,       nullptr},
        {"memcpy",      (PyCFunction) Unsafe_memcpy,     METH_VARARGS, nullptr},
        {"incref",      (PyCFunction) Unsafe_incref,     METH_O,       nullptr},
        {"decref",      (PyCFunction) Unsafe_decref,     METH_O,       nullptr},
        {"refcnt",      (PyCFunction) Unsafe_refcnt,     METH_O,       nullptr},
        {"fputs",       (PyCFunction) Unsafe_fputs,      METH_O,       nullptr},
        {"fflush",      (PyCFunction) Unsafe_fflush,     METH_NOARGS,  nullptr},
        {"fgets",       (PyCFunction) Unsafe_fgets,      METH_O,       nullptr},
        {nullptr,       nullptr, 0,                                    nullptr}
};


void initializeUnsafeType(PyTypeObject &type) {
    type.tp_name = "Unsafe";
    type.tp_basicsize = sizeof(Unsafe);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_methods = Unsafe_methods;
    type.tp_init = (initproc) Unsafe_init;
    type.tp_new = PyType_GenericNew;
    type.tp_dealloc = (destructor) Unsafe_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
}

static struct PyModuleDef Unsafe_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.Unsafe",
        "Allow access to C level APIs.",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_Unsafe() {
    initializeUnsafeType(UnsafeType);

    PyObject *object = PyModule_Create(&Unsafe_module);
    if (object == nullptr)
        return nullptr;

    Py_INCREF(&UnsafeType);
    if (PyModule_AddObject(object, "Unsafe", (PyObject *) &UnsafeType) < 0) {
        Py_DECREF(&UnsafeType);
        Py_DECREF(object);
        return nullptr;
    }

    return object;
}
#pragma clang diagnostic pop
}