//
// Created by xia__mc on 2024/11/9.
//

#ifndef PYFASTUTIL_PYTHONUTILS_H
#define PYFASTUTIL_PYTHONUTILS_H

#include <iostream>
#include "PythonPCH.h"
#include "Compat.h"

template<typename T>
static __forceinline constexpr void PyFast_INCREF(T *object) noexcept {
    if (UNLIKELY(((PyObject *) object)->ob_refcnt < 0))
        return;
    ((PyObject *) object)->ob_refcnt++;
}

template<typename T>
static __forceinline constexpr void PyFast_DECREF(T *object) noexcept {
    if (UNLIKELY(((PyObject *) object)->ob_refcnt < 0))
        return;
    if (--((PyObject *) object)->ob_refcnt == 0)
        _Py_Dealloc((PyObject *) object);
}

template<typename T>
static __forceinline constexpr void PyFast_XDECREF(T *object) noexcept {
    if (UNLIKELY(object == nullptr))
        return;
    if (UNLIKELY(((PyObject *) object)->ob_refcnt < 0))
        return;
    if (--((PyObject *) object)->ob_refcnt == 0)
        _Py_Dealloc((PyObject *) object);
}

template<typename T>
static __forceinline constexpr void SAFE_DECREF(T *&object) noexcept {
    assert(object != nullptr);
    PyFast_DECREF(object);
    object = nullptr;
}

template<typename T>
static __forceinline T *Py_CreateObj(PyTypeObject &typeObj) noexcept {
    return (T *) PyObject_CallObject((PyObject *) &typeObj, nullptr);
}

template<typename T>
static __forceinline T *Py_CreateObj(PyTypeObject &typeObj, PyObject *args) noexcept {
    return (T *) PyObject_CallObject((PyObject *) &typeObj, args);
}

template<typename T>
static __forceinline T *Py_CreateObjNoInit(PyTypeObject &typeObj) noexcept {
    return (T *) PyObject_New(T, &typeObj);
}

/**
 * Try to parse args like built-in range function
 * If not successful, function will raise python exception.
 * @return if successful
 */
static __forceinline bool PyParse_EvalRange(PyObject *&args,
                                            Py_ssize_t &start, Py_ssize_t &stop, Py_ssize_t &step) noexcept {
    Py_ssize_t arg1 = PY_SSIZE_T_MAX;
    Py_ssize_t arg2 = PY_SSIZE_T_MAX;
    Py_ssize_t arg3 = PY_SSIZE_T_MAX;

    if (!PyArg_ParseTuple(args, "n|nn", &arg1, &arg2, &arg3)) {
        return false;
    }

    if (arg2 == PY_SSIZE_T_MAX) {
        start = 0;
        stop = arg1;
        step = 1;
    } else if (arg3 == PY_SSIZE_T_MAX) {
        start = arg1;
        stop = arg2;
        step = 1;
    } else if (arg3 != 0) {
        start = arg1;
        stop = arg2;
        step = arg3;
    } else {
        PyErr_SetString(PyExc_ValueError, "Arg 3 must not be zero.");
        return false;
    }

    return true;
}

#define Py_RETURN_BOOL(b) { if (b) Py_RETURN_TRUE; else Py_RETURN_FALSE; }

#ifdef IS_PYTHON_312_OR_LATER
#define PyLong_DIGITS(obj) ((obj)->long_value.ob_digit)
#else
#define PyLong_DIGITS(obj) ((obj)->ob_digit)
#endif

#ifdef IS_PYTHON_312_OR_LATER
#define PyLong_TAGS(obj) ((obj)->long_value.lv_tag)
#else
#define PyLong_TAGS(obj) ((obj)->lv_tag)
#endif

static __forceinline int PyFast_AsInt(PyObject *obj) noexcept {
#ifdef IS_PYTHON_313_OR_LATER
    return PyLong_AsInt(obj);
#else
#ifdef IS_PYTHON_312_OR_LATER
    return _PyLong_AsInt(obj);
#else
    const Py_ssize_t size = Py_SIZE(obj);
    if (size == 0) {
        return 0;
    } else if (size < 0) {
        return -(int) *PyLong_DIGITS((PyLongObject *) obj);
    } else {
        return (int) *PyLong_DIGITS((PyLongObject *) obj);
    }
#endif
#endif
}

static __forceinline short PyFast_AsShort(const PyObject *__restrict obj) noexcept {
    const Py_ssize_t size = Py_SIZE(obj);
    if (size == 0) {
        return 0;
    } else if (size < 0) {
        return (short) (-(int) *PyLong_DIGITS((PyLongObject *) obj));
    } else {
        return (short) *PyLong_DIGITS((PyLongObject *) obj);
    }
}

static __forceinline char PyFast_AsChar(const PyObject *__restrict obj) noexcept {
    const Py_ssize_t size = Py_SIZE(obj);
    if (size == 0) {
        return 0;
    } else if (size < 0) {
        return (char) (-(int) *PyLong_DIGITS((PyLongObject *) obj));
    } else {
        return (char) *PyLong_DIGITS((PyLongObject *) obj);
    }
}

static __forceinline PyObject *PyFast_FromInt(const int value) noexcept {
#ifdef IS_PYTHON_312_OR_LATER
    return (PyObject *) _PyLong_FromDigits(value < 0, 1, (digit *) (&value));
#else
    return PyLong_FromLong(value);
#endif
}


// API for native code generator
[[maybe_unused]] static __forceinline PyObject *PyFast_FromDigits(
        [[maybe_unused]] const char *string,
        [[maybe_unused]] const int negative,
        [[maybe_unused]] const int digit_count,
        [[maybe_unused]] const std::initializer_list<digit> &digits) noexcept {
#ifdef IS_PYTHON_312_OR_LATER
    return (PyObject *) _PyLong_FromDigits(negative, digit_count, (digit *) data(digits));
#else
    return PyLong_FromString(string, nullptr, 10);
#endif
}

[[maybe_unused]] static __forceinline PyObject *PyFast_LoadGlobal(PyObject *name, Py_hash_t hash) noexcept {
    PyThreadState *threadState = PyThreadState_Get();
    PyFrameObject *frame = PyThreadState_GetFrame(threadState);
    PyObject *globals = PyFrame_GetGlobals(frame);
    PyObject *builtins = PyFrame_GetBuiltins(frame);

#ifdef IS_PYTHON_312_OR_LATER
    PyObject *value = _PyDict_GetItem_KnownHash(globals, name, hash);
    if (value == nullptr) {
        value = _PyDict_GetItem_KnownHash(builtins, name, hash);
    }
#else
    PyObject *value = PyDict_GetItem(globals, name);
    if (value == nullptr) {
        value = PyDict_GetItem(builtins, name);
    }
#endif

    if (value != nullptr) {
        Py_INCREF(value);
    }
    return value;
}

[[maybe_unused]] static __forceinline PyObject *PyFast_TuplePack(
        const std::initializer_list<PyObject *> &list, const size_t size) noexcept {
    auto *result = PyTuple_New((Py_ssize_t) size);

    memcpy(((PyTupleObject *) result)->ob_item, data(list), size * sizeof(PyObject *));
    return result;
}

[[maybe_unused]] static __forceinline PyObject *PyFast_DictPackWithTuple(
        PyObject *keys, PyObject **values, const size_t size,
        [[maybe_unused]] const std::initializer_list<Py_hash_t> &keysHash) noexcept {
    auto *result = PyDict_New();

    auto keysIter = ((PyTupleObject *) keys)->ob_item;
#ifdef IS_PYTHON_312_OR_LATER
    auto keysHashIter = keysHash.begin();
#endif
    for (size_t i = 0; i < size; ++i) {
#ifdef IS_PYTHON_312_OR_LATER
        _PyDict_SetItem_KnownHash(result, *keysIter, *values, *keysHashIter);
#else
        PyDict_SetItem(result, *keysIter, *valuesIter);
#endif
        keysIter++;
        values++;
#ifdef IS_PYTHON_312_OR_LATER
        keysHashIter++;
#endif
    }

    return result;
}

[[maybe_unused]] static __forceinline PyObject *PyFast_GetIter(PyObject *&obj) noexcept {
    const auto type = obj->ob_type;
    if (LIKELY(type == &PyList_Type)) {
        return PyList_Type.tp_iter(obj);
    } else if (type == &PyTuple_Type) {
        return PyTuple_Type.tp_iter(obj);
    } else if (type == &PyDict_Type) {
        return PyDict_Type.tp_iter(obj);
    } else if (type == &PySet_Type) {
        return PySet_Type.tp_iter(obj);
    } else if (UNLIKELY(type == &PyUnicode_Type)) {
        return PyUnicode_Type.tp_iter(obj);
    }

    return PyObject_GetIter(obj);
}

#define GET_NARGS_F(type, nargs) (type == &PyMethod_Type ? nargs | PY_VECTORCALL_ARGUMENTS_OFFSET : nargs)

static const PyObject *EMPTY_TUPLE = PyTuple_New(0);
#define EMPTY_TUPLE ((PyObject *) EMPTY_TUPLE)

[[maybe_unused]] static __forceinline PyObject *PyFast_CallNoArgs(PyObject *callable) noexcept {
    const PyTypeObject *type = callable->ob_type;
    const vectorcallfunc vecCallable = type->tp_vectorcall;
    if (LIKELY(vecCallable != nullptr)) {
        return vecCallable(callable, nullptr, GET_NARGS_F(type, 0), nullptr);
    }
    if (LIKELY(type->tp_flags & _Py_TPFLAGS_HAVE_VECTORCALL)) {
        return (*(vectorcallfunc *) (((char *) callable) + type->tp_vectorcall_offset))(
                callable, nullptr, GET_NARGS_F(type, 0), nullptr);
    }

    // fallback
    PyObject *result = type->tp_call(callable, EMPTY_TUPLE, nullptr);
    return result;
}

[[maybe_unused]] static __forceinline PyObject *PyFast_CallNoKwargs(
        PyObject *callable, const std::initializer_list<PyObject *> &args) noexcept {
    const PyTypeObject *type = callable->ob_type;
    const vectorcallfunc vecCallable = type->tp_vectorcall;
    const size_t argsCount = args.size();
    if (LIKELY(vecCallable != nullptr)) {
        return vecCallable(callable, data(args), GET_NARGS_F(type, argsCount), nullptr);
    }
    if (LIKELY(type->tp_flags & _Py_TPFLAGS_HAVE_VECTORCALL)) {
        return (*(vectorcallfunc *) (((char *) callable) + type->tp_vectorcall_offset))(
                callable, data(args), GET_NARGS_F(type, argsCount), nullptr);
    }

    // fallback
    PyObject *argsTuple = PyFast_TuplePack(args, argsCount);
    PyObject *result = type->tp_call(callable, argsTuple, nullptr);
    Py_DECREF(argsTuple);
    return result;
}

[[maybe_unused]] static __forceinline PyObject *PyFast_Call(
        PyObject *callable, const std::initializer_list<PyObject *> &args,
        const size_t posArgCount, PyObject *kwNames, const std::initializer_list<Py_hash_t> &kwNamesHash) noexcept {
    const auto type = callable->ob_type;
    const vectorcallfunc vecCallable = type->tp_vectorcall;
    if (LIKELY(vecCallable != nullptr)) {
        return vecCallable(callable, data(args), GET_NARGS_F(type, posArgCount), kwNames);
    }
    if (LIKELY(type->tp_flags & _Py_TPFLAGS_HAVE_VECTORCALL)) {
        return (*(vectorcallfunc *) (((char *) callable) + type->tp_vectorcall_offset))(
                callable, data(args), GET_NARGS_F(type, posArgCount), kwNames);
    }

    // fallback
    PyObject *argsTuple = PyFast_TuplePack(args, posArgCount);
    PyObject *kwargs = PyFast_DictPackWithTuple(kwNames, (PyObject **) data(args) + posArgCount,
                                                args.size() - posArgCount, kwNamesHash);
    PyObject *result = type->tp_call(callable, argsTuple, kwargs);
    Py_DECREF(argsTuple);
    Py_DECREF(kwargs);
    return result;
}

[[maybe_unused]] static __forceinline PyObject *PyFast_Next(PyObject *iterable) noexcept {
    return (*iterable->ob_type->tp_iternext)(iterable);
}


#endif //PYFASTUTIL_PYTHONUTILS_H
