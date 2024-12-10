//
// Created by xia__mc on 2024/12/10.
//

#include "ASM.h"
#include "utils/memory/AlignedAllocator.h"
#include "utils/memory/FastMemcpy.h"
#include "Compat.h"

#ifdef WINDOWS

#include "windows.h"

#endif

extern "C" {

static PyTypeObject ASMType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
};

static int ASM_init([[maybe_unused]] ASM *self,
                    [[maybe_unused]] PyObject *args, [[maybe_unused]] PyObject *kwargs) {
    return 0;
}

static void ASM_dealloc(ASM *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *ASM_enter(PyObject *self, [[maybe_unused]] PyObject *args) {
    Py_INCREF(self);
    return self;
}

static PyObject *ASM_exit([[maybe_unused]] PyObject *self,
                          [[maybe_unused]] PyObject *const *args, [[maybe_unused]] Py_ssize_t nargs) {
    Py_RETURN_NONE;
}

static PyObject *ASM_run([[maybe_unused]] PyObject *__restrict self,
                         PyObject *const *__restrict args, Py_ssize_t nargs) noexcept {
#ifdef WINDOWS
    if (nargs != 1) {
        PyErr_SetString(PyExc_TypeError, "Function takes exactly 1 arguments (__code)");
        return nullptr;
    }

    if (!PyBytes_Check(args[0])) {
        PyErr_SetString(PyExc_TypeError, "Expected a bytes object.");
        return nullptr;
    }

    const auto *__restrict code = (const unsigned char *) PyBytes_AsString(args[0]);
    if (code == nullptr) {
        PyErr_SetString(PyExc_ValueError, "Failed to convert argument to c str.");
        return nullptr;
    }

    const auto size = (size_t) PyBytes_Size(args[0]) * sizeof(unsigned char);

    unsigned char *__restrict target;
    const bool aligned = (uintptr_t) code % 16 == 0;
    if (!aligned) {
        try {
            target = (unsigned char *) alignedAlloc(size, 16);
        } catch (const std::exception &e) {
            PyErr_SetString(PyExc_MemoryError, e.what());
            return nullptr;
        }
        fast_memcpy(target, code, size);
    } else {
        target = (unsigned char *) code;
    }

    DWORD oldProtect;
    if (!VirtualProtect((LPVOID) target, size,
                        PAGE_EXECUTE_READWRITE, &oldProtect)) {
        PyErr_SetString(PyExc_OSError, "Failed to make ASM executable.");

        if (!aligned) {
            alignedFree(target);
        }
        return nullptr;
    }

    ((void (*)()) target)();

    if (!VirtualProtect((LPVOID) target, size, oldProtect, &oldProtect)) {
        PyErr_SetString(PyExc_OSError, "Failed to restore memory protection.");

        if (!aligned) {
            alignedFree(target);
        }
        return nullptr;
    }

    if (!aligned) {
        alignedFree(target);
    }

    Py_RETURN_NONE;
#else
    PyErr_SetString(PyExc_NotImplementedError, "ASM is not supported on this architecture.");
    return nullptr;
#endif
}

static PyObject *ASM_runFast([[maybe_unused]] PyObject *__restrict self,
                             PyObject *const *__restrict args, [[maybe_unused]] Py_ssize_t nargs) noexcept {
#ifdef WINDOWS
    const auto *__restrict code = (const unsigned char *) PyBytes_AS_STRING(args[0]);
    const auto size = (size_t) PyBytes_GET_SIZE(args[0]) * sizeof(unsigned char);

    if ((uintptr_t) code % 16 != 0) {
        unsigned char *__restrict target;
        try {
            target = (unsigned char *) alignedAlloc(size, 16);
        } catch (const std::exception &e) {
            PyErr_SetString(PyExc_MemoryError, e.what());
            return nullptr;
        }
        fast_memcpy(target, code, size);

        DWORD oldProtect;
        if (!VirtualProtect((LPVOID) target, size,
                            PAGE_EXECUTE_READWRITE, &oldProtect)) {
            PyErr_SetString(PyExc_OSError, "Failed to make ASM executable.");
            alignedFree(target);
            return nullptr;
        }

        ((void (*)()) target)();

        if (!VirtualProtect((LPVOID) target, size, oldProtect, &oldProtect)) {
            PyErr_SetString(PyExc_OSError, "Failed to restore memory protection.");
            alignedFree(target);
            return nullptr;
        }

        alignedFree(target);
    } else {
        DWORD oldProtect;
        if (!VirtualProtect((LPVOID) code, size,
                            PAGE_EXECUTE_READWRITE, &oldProtect)) {
            PyErr_SetString(PyExc_OSError, "Failed to make ASM executable.");
            return nullptr;
        }

        ((void (*)()) code)();

        if (!VirtualProtect((LPVOID) code, size, oldProtect, &oldProtect)) {
            PyErr_SetString(PyExc_OSError, "Failed to restore memory protection.");
            return nullptr;
        }
    }

    Py_RETURN_NONE;
#else
    PyErr_SetString(PyExc_NotImplementedError, "ASM is not supported on this architecture.");
    return nullptr;
#endif
}

static PyMethodDef ASM_methods[] = {
        {"__enter__", (PyCFunction) ASM_enter, METH_NOARGS, nullptr},
        {"__exit__", (PyCFunction) ASM_exit, METH_FASTCALL, nullptr},
        {"run", (PyCFunction) ASM_run, METH_FASTCALL, nullptr},
        {"runFast", (PyCFunction) ASM_runFast, METH_FASTCALL, nullptr},
        {nullptr, nullptr, 0, nullptr}
};

void initializeASMType(PyTypeObject &type) {
    type.tp_name = "ASM";
    type.tp_basicsize = sizeof(ASM);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_methods = ASM_methods;
    type.tp_init = (initproc) ASM_init;
    type.tp_new = PyType_GenericNew;
    type.tp_dealloc = (destructor) ASM_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
}

static struct PyModuleDef ASM_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.ASM",
        "Allow access to native asm.",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_ASM() {
    initializeASMType(ASMType);

    PyObject *object = PyModule_Create(&ASM_module);
    if (object == nullptr)
        return nullptr;

    Py_INCREF(&ASMType);
    if (PyModule_AddObject(object, "ASM", (PyObject *) &ASMType) < 0) {
        Py_DECREF(&ASMType);
        Py_DECREF(object);
        return nullptr;
    }

    return object;
}
#pragma clang diagnostic pop

}