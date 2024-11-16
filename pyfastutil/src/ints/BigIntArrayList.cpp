//
// Created by xia__mc on 2024/11/16.
//

#include "BigIntArrayList.h"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "utils/PythonUtils.h"
#include "utils/TimSort.h"
#include "utils/simd/BitonicSort.h"
#include "utils/simd/Utils.h"
#include "utils/memory/AlignedAllocator.h"
#include "ints/BigIntArrayListIter.h"

extern "C" {

static PyTypeObject BigIntArrayListType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
};

__forceinline void parseArgs(PyObject *&args, PyObject *&kwargs, PyObject *&pyIterable, Py_ssize_t &pySize) {
    static constexpr const char *kwlist[] = {"iterable", "exceptSize", nullptr};

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

static int BigIntArrayList_init(BigIntArrayList *self, PyObject *args, PyObject *kwargs) {
    new(&self->vector) std::vector<long long, AlignedAllocator<long long, 64>>();

    PyObject *pyIterable = nullptr;
    Py_ssize_t pySize = -1;

    parseArgs(args, kwargs, pyIterable, pySize);

    // init vector
    try {
        if (pySize > 0) {
            self->vector.reserve(static_cast<size_t>(pySize));
        }

        if (pyIterable != nullptr) {
            if (Py_TYPE(pyIterable) == &BigIntArrayListType) {  // BigIntArrayList is a final class
                auto *iter = reinterpret_cast<BigIntArrayList *>(pyIterable);
                self->vector = iter->vector;
                return 0;
            }

            PyObject *iter = PyObject_GetIter(pyIterable);
            if (iter == nullptr) {
                PyErr_SetString(PyExc_TypeError, "Arg '__iterable' is not iterable.");
                return -1;
            }

            PyObject *item;
            while ((item = PyIter_Next(iter)) != nullptr) {
                long long value = PyLong_AsLongLong(item);
                if (PyErr_Occurred()) {
                    SAFE_DECREF(iter);
                    SAFE_DECREF(item);
                    PyErr_SetString(PyExc_RuntimeError, "Failed to convert item to C long long during iteration.");
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

static void BigIntArrayList_dealloc(BigIntArrayList *self) {
    self->vector.~vector();
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *BigIntArrayList_from_range([[maybe_unused]] PyObject *cls, PyObject *args) {
    Py_ssize_t start, stop, step;

    if (!PyParse_EvalRange(args, start, stop, step)) {
        return nullptr;
    }

    auto *list = Py_CreateObj<BigIntArrayList>(BigIntArrayListType);
    if (list == nullptr) return PyErr_NoMemory();

    try {
        Py_ssize_t elements;
        if (step > 0) {
            if (start >= stop) {
                return reinterpret_cast<PyObject *>(list);
            }
            elements = (stop - start + step - 1) / step;
        } else { // step < 0
            if (start <= stop) {
                return reinterpret_cast<PyObject *>(list);
            }
            elements = ((start - stop) / (-step));
        }

        list->vector.resize(elements);

        Py_ssize_t current = start;
        for (Py_ssize_t idx = 0; idx < elements; ++idx) {
            list->vector[idx] = static_cast<long long>(current);
            current += step;
        }

    } catch (const std::exception &e) {
        list->vector.~vector();
        PyObject_Del(list);
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    return reinterpret_cast<PyObject *>(list);
}

static PyObject *BigIntArrayList_resize(PyObject *pySelf, PyObject *pySize) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    if (!PyLong_Check(pySize)) {
        PyErr_SetString(PyExc_TypeError, "Expected an int object.");
        return nullptr;
    }

    Py_ssize_t pySSize = PyLong_AsSsize_t(pySize);
    if (pySSize < 0) {
        PyErr_SetString(PyExc_ValueError, "Invalid size.");
        return nullptr;
    }

    try {
        self->vector.resize(static_cast<size_t>(pySSize));
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    Py_RETURN_NONE;
}

static PyObject *BigIntArrayList_to_list(PyObject *pySelf) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    const auto size = static_cast<Py_ssize_t>(self->vector.size());
    PyObject *result = PyList_New(size);
    if (result == nullptr) return PyErr_NoMemory();

    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject *item = PyLong_FromLongLong(self->vector[i]);
        if (item == nullptr) {
            SAFE_DECREF(result);
            return nullptr;
        }

        PyList_SET_ITEM(result, i, item);  // PyList_SET_ITEM handle this ref
    }

    return result;
}

static PyObject *BigIntArrayList_copy(PyObject *pySelf) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    auto *copy = Py_CreateObj<BigIntArrayList>(BigIntArrayListType);
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

static PyObject *BigIntArrayList_append(PyObject *pySelf, PyObject *object) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    long long value = PyLong_AsLongLong(object);
    if (PyErr_Occurred()) {
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

static PyObject *BigIntArrayList_extend(PyObject *pySelf, PyObject *const *args, const Py_ssize_t nargs) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    // FASTCALL ensure args != nullptr
    if (nargs != 1) {
        PyErr_SetString(PyExc_TypeError, "extend() takes exactly one argument");
        return nullptr;
    }

    PyObject *iterable = args[0];

    // fast extend
    if (Py_TYPE(iterable) == &BigIntArrayListType) {
        auto *iter = reinterpret_cast<BigIntArrayList *>(iterable);
        self->vector.insert(self->vector.end(), iter->vector.begin(), iter->vector.end());
        Py_RETURN_NONE;
    }

    // python iterable extend
    PyObject *iter = PyObject_GetIter(iterable);
    if (iter == nullptr) {
        return nullptr;
    }

    // pre alloc
    Py_ssize_t hint = PyObject_LengthHint(iterable, 0);
    if (hint > 0) {
        self->vector.reserve(self->vector.size() + hint);
    }

    // do extend
    PyObject *item;
    while ((item = PyIter_Next(iter)) != nullptr) {
        long long value = PyLong_AsLongLong(item);
        SAFE_DECREF(item);

        if (PyErr_Occurred()) {
            SAFE_DECREF(iter);
            return nullptr;
        }

        self->vector.push_back(value);
    }

    if (PyErr_Occurred()) {
        SAFE_DECREF(iter);
        return nullptr;
    }

    SAFE_DECREF(iter);
    Py_RETURN_NONE;
}


static PyObject *BigIntArrayList_pop(PyObject *pySelf, PyObject *const *args, const Py_ssize_t nargs) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    const auto vecSize = static_cast<Py_ssize_t>(self->vector.size());
    Py_ssize_t index = vecSize - 1;

    if (nargs == 1) {
        index = PyLong_AsSsize_t(args[0]);
        if (index == -1 && PyErr_Occurred()) {
            return nullptr;
        }

        if (index < 0) {
            index += vecSize;
        }

        if (index < 0 || index >= vecSize) {
            PyErr_SetString(PyExc_IndexError, "index out of range");
            return nullptr;
        }
    } else if (nargs > 1) {
        PyErr_SetString(PyExc_TypeError, "pop() takes at most 1 argument");
        return nullptr;
    } else {
        const auto popped = self->vector[static_cast<size_t>(index)];
        self->vector.pop_back();

        return PyLong_FromLongLong(popped);
    }

    const auto popped = self->vector[static_cast<size_t>(index)];
    self->vector.erase(self->vector.begin() + index);

    return PyLong_FromLongLong(popped);
}

static PyObject *BigIntArrayList_index(PyObject *pySelf, PyObject *args) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    long long value;
    Py_ssize_t start = 0;
    auto stop = static_cast<Py_ssize_t>(self->vector.size());

    if (!PyArg_ParseTuple(args, "L|nn", &value, &start, &stop)) {
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

static PyObject *BigIntArrayList_count(PyObject *pySelf, PyObject *object) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    long long value = PyLong_AsLongLong(object);
    if (value == -1 && PyErr_Occurred()) {
        return nullptr;
    }

    try {
        size_t result = std::count(self->vector.begin(), self->vector.end(), value);

        return PyLong_FromSize_t(result);
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static PyObject *BigIntArrayList_insert(PyObject *pySelf, PyObject *args) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    Py_ssize_t index;
    long long value;

    if (!PyArg_ParseTuple(args, "nL", &index, &value)) {
        return nullptr;
    }

    // fix index
    const auto vecSize = static_cast<Py_ssize_t>(self->vector.size());
    if (index < 0) {
        index = std::max(static_cast<Py_ssize_t>(0), vecSize + index);
    } else if (index > vecSize) {
        index = vecSize;
    }

    // do insert
    try {
        self->vector.insert(self->vector.begin() + index, value);
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    Py_RETURN_NONE;
}

static PyObject *BigIntArrayList_remove(PyObject *pySelf, PyObject *object) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    long long value = PyLong_AsLongLong(object);
    if (PyErr_Occurred()) {
        return nullptr;
    }

    try {
        auto it = std::find(self->vector.begin(), self->vector.end(), value);

        if (it != self->vector.end()) {
            self->vector.erase(it);
        } else {
            PyErr_SetString(PyExc_ValueError, "Value is not in list.");
            return nullptr;
        }
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    Py_RETURN_NONE;
}

static PyObject *BigIntArrayList_sort(PyObject *pySelf, PyObject *args, PyObject *kwargs) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    PyObject *keyFunc = nullptr;
    long long reverseInt = 0;  // default: false
    static const char *kwlist[] = {"key", "reverse", nullptr};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OL", const_cast<char **>(kwlist), &keyFunc, &reverseInt)) {
        return nullptr;
    }

    const bool reverse = reverseInt == 1;

    // do sort
    try {
        if (keyFunc == nullptr || keyFunc == Py_None) {
            if (self->vector.size() >= 32) {
                // simd sort with auto-fallback
                Py_BEGIN_ALLOW_THREADS
                    simd::simdsort(self->vector, reverse);
                Py_END_ALLOW_THREADS
            } else {
                if (reverse) {
                    std::sort(self->vector.begin(), self->vector.end(), std::greater<>());
                } else {
                    std::sort(self->vector.begin(), self->vector.end());
                }
            }
        } else {
            // sort with key function
            const auto keyWrapper = [keyFunc](long long value) -> PyObject * {
                // call key function
                PyObject *arg = PyLong_FromLongLong(value);
                if (arg == nullptr) {
                    throw std::runtime_error("Failed to create argument for key function.");
                }
                PyObject *result = PyObject_CallFunctionObjArgs(keyFunc, arg, nullptr);
                SAFE_DECREF(arg);
                if (result == nullptr) {
                    throw std::runtime_error("Key function call failed.");
                }
                return result;
            };

            const auto cmpWrapper = [&keyWrapper](long long a, long long b) -> bool {
                // do comp
                PyObject *keyA = keyWrapper(a);
                PyObject *keyB = keyWrapper(b);
                int cmpResult = PyObject_RichCompareBool(keyA, keyB, Py_LT);  // a < b ?
                SAFE_DECREF(keyA);
                SAFE_DECREF(keyB);
                if (cmpResult == -1) {
                    throw std::runtime_error("Failed to comparison.");
                }
                return cmpResult == 1;
            };

            if (self->vector.size() < 5000) {
                if (reverse) {
                    std::sort(self->vector.begin(), self->vector.end(), [cmpWrapper](long long a, long long b) {
                        return cmpWrapper(b, a);  // reverse
                    });
                } else {
                    std::sort(self->vector.begin(), self->vector.end(), cmpWrapper);
                }
            } else {
                if (reverse) {
                    gfx::timsort(self->vector.begin(), self->vector.end(), [cmpWrapper](long long a, long long b) {
                        return cmpWrapper(b, a);  // reverse
                    });
                } else {
                    gfx::timsort(self->vector.begin(), self->vector.end(), cmpWrapper);
                }
            }
        }
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    Py_RETURN_NONE;
}

static Py_ssize_t BigIntArrayList_len(PyObject *pySelf) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    return static_cast<Py_ssize_t>(self->vector.size());
}


static PyObject *BigIntArrayList_iter(PyObject *pySelf) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    auto iter = BigIntArrayListIter_create(self);
    if (iter == nullptr) return PyErr_NoMemory();
    return reinterpret_cast<PyObject *>(iter);
}

static PyObject *BigIntArrayList_getitem(PyObject *pySelf, Py_ssize_t pyIndex) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    auto size = static_cast<Py_ssize_t>(self->vector.size());

    if (pyIndex < 0) {
        pyIndex = size + pyIndex;
    }

    if (pyIndex < 0 || pyIndex >= size) {
        PyErr_SetString(PyExc_IndexError, "index out of range.");
        return nullptr;
    }

    try {
        return PyLong_FromLongLong(self->vector[static_cast<size_t>(pyIndex)]);
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static PyObject *BigIntArrayList_getitem_slice(PyObject *pySelf, PyObject *slice) {
    if (PyIndex_Check(slice)) {
        Py_ssize_t pyIndex = PyNumber_AsSsize_t(slice, PyExc_IndexError);
        if (pyIndex == -1 && PyErr_Occurred()) {
            return nullptr;
        }
        return BigIntArrayList_getitem(pySelf, pyIndex);
    }

    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    Py_ssize_t start, stop, step, sliceLength;
    if (PySlice_Unpack(slice, &start, &stop, &step) < 0) {
        return nullptr;
    }

    sliceLength = PySlice_AdjustIndices(static_cast<Py_ssize_t>(self->vector.size()), &start, &stop, step);

    PyObject *result = PyList_New(sliceLength);
    if (!result) {
        return nullptr;
    }

    for (Py_ssize_t i = 0; i < sliceLength; i++) {
        Py_ssize_t index = start + i * step;
        PyObject *item = PyLong_FromLongLong(self->vector[static_cast<size_t>(index)]);
        if (item == nullptr) {
            SAFE_DECREF(result);
            return nullptr;
        }
        PyList_SET_ITEM(result, i, item);
    }
    return result;
}

static int BigIntArrayList_setitem(PyObject *pySelf, Py_ssize_t pyIndex, PyObject *pyValue) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    auto size = static_cast<Py_ssize_t>(self->vector.size());

    if (pyIndex < 0) {
        pyIndex = size + pyIndex;
    }
    if (pyIndex < 0 || pyIndex >= size) {
        PyErr_SetString(PyExc_IndexError, "index out of range.");
        return -1;
    }

    try {
        if (pyValue == nullptr) {
            self->vector.erase(self->vector.begin() + pyIndex);
        } else {
            long long value = PyLong_AsLongLong(pyValue);
            if (PyErr_Occurred()) {
                return -1;
            }

            self->vector[static_cast<size_t>(pyIndex)] = value;
        }
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }
    return 0;
}


static int BigIntArrayList_setitem_slice(PyObject *pySelf, PyObject *slice, PyObject *value) {
    if (PyIndex_Check(slice)) {
        Py_ssize_t index = PyNumber_AsSsize_t(slice, PyExc_IndexError);
        if (index == -1 && PyErr_Occurred()) {
            return -1;
        }
        return BigIntArrayList_setitem(pySelf, index, value);
    }

    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    Py_ssize_t start, stop, step, sliceLength;
    if (PySlice_Unpack(slice, &start, &stop, &step) < 0) {
        return -1;
    }

    sliceLength = PySlice_AdjustIndices(static_cast<Py_ssize_t>(self->vector.size()), &start, &stop, step);

    if (!PySequence_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "can only assign an iterable");
        return -1;
    }

    if (PySequence_Size(value) != sliceLength) {
        PyErr_SetString(PyExc_ValueError, "attempt to assign sequence of size different from slice");
        return -1;
    }

    PyObject *item = nullptr;
    try {
        for (Py_ssize_t i = 0; i < sliceLength; i++) {
            Py_ssize_t index = start + i * step;

            if (value == nullptr) {
                self->vector.erase(self->vector.begin() + index);
            } else {
                item = PySequence_GetItem(value, i);
                if (item == nullptr) {
                    return -1;
                }

                self->vector[static_cast<size_t>(index)] = PyLong_AsLongLong(item);
                if (PyErr_Occurred()) {
                    SAFE_DECREF(item);
                    return -1;
                }

                SAFE_DECREF(item);
            }
        }
    } catch (const std::exception &e) {
        SAFE_DECREF(item);
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }
    return 0;
}

static PyObject *BigIntArrayList_add(PyObject *pySelf, PyObject *pyValue) {
    if (Py_TYPE(pyValue) == &BigIntArrayListType) {
        // fast add -> BigIntArrayList
        auto *value = reinterpret_cast<BigIntArrayList *>(pyValue);

        auto *result = Py_CreateObj<BigIntArrayList>(BigIntArrayListType, pySelf);
        if (result == nullptr) {
            return PyErr_NoMemory();
        }

        try {
            result->vector.insert(result->vector.end(), value->vector.begin(), value->vector.end());
            return reinterpret_cast<PyObject *>(result);
        } catch (const std::exception &e) {
            SAFE_DECREF(result);
            PyErr_SetString(PyExc_RuntimeError, e.what());
            return nullptr;
        }
    }

    // add -> list[int]
    PyObject *selfIntList = BigIntArrayList_to_list(pySelf);
    if (selfIntList == nullptr) {
        return nullptr;
    }

    PyObject *result = PySequence_Concat(selfIntList, pyValue);
    SAFE_DECREF(selfIntList);

    if (result == nullptr) {
        return nullptr;
    }

    return result;
}

static PyObject *BigIntArrayList_iadd(PyObject *pySelf, PyObject *iterable) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    // fast extend
    if (Py_TYPE(iterable) == &BigIntArrayListType) {
        auto *iter = reinterpret_cast<BigIntArrayList *>(iterable);
        self->vector.insert(self->vector.end(), iter->vector.begin(), iter->vector.end());
        Py_RETURN_NONE;
    }

    // python iterable extend
    PyObject *iter = PyObject_GetIter(iterable);
    if (iter == nullptr) {
        return nullptr;
    }

    // pre alloc
    Py_ssize_t hint = PyObject_LengthHint(iterable, 0);
    if (hint > 0) {
        self->vector.reserve(self->vector.size() + hint);
    }

    // do extend
    PyObject *item;
    while ((item = PyIter_Next(iter)) != nullptr) {
        long long value = PyLong_AsLongLong(item);
        SAFE_DECREF(item);

        if (PyErr_Occurred()) {
            SAFE_DECREF(iter);
            return nullptr;
        }

        self->vector.push_back(value);
    }

    if (PyErr_Occurred()) {
        SAFE_DECREF(iter);
        return nullptr;
    }

    SAFE_DECREF(iter);
    Py_INCREF(pySelf);
    return pySelf;
}


static PyObject *BigIntArrayList_mul(PyObject *pySelf, Py_ssize_t n) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    if (n < 0) {
        n = 0;
    }

    auto *result = Py_CreateObj<BigIntArrayList>(BigIntArrayListType);
    if (result == nullptr) {
        return PyErr_NoMemory();
    }

    if (n == 0) {
        return reinterpret_cast<PyObject *>(result);
    }

    try {
        const auto selfSize = self->vector.size();

        Py_BEGIN_ALLOW_THREADS
            result->vector.resize(selfSize * n);
            for (Py_ssize_t i = 0; i < n; ++i) {
                simd::simdMemCpyAligned(self->vector.data(), result->vector.data() + selfSize * i, selfSize);
            }
        Py_END_ALLOW_THREADS

        return reinterpret_cast<PyObject *>(result);
    } catch (const std::exception &e) {
        SAFE_DECREF(result);
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static PyObject *BigIntArrayList_rmul(PyObject *pySelf, PyObject *pyValue) {
    if (PyLong_Check(pyValue)) {
        Py_ssize_t n = PyLong_AsSsize_t(pyValue);
        if (PyErr_Occurred()) {
            return nullptr;
        }

        return BigIntArrayList_mul(pySelf, n);
    }

    PyErr_SetString(PyExc_TypeError, "Expected an integer on the left-hand side of *");
    return nullptr;
}

static PyObject *BigIntArrayList_imul(PyObject *pySelf, Py_ssize_t n) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    if (n < 0) {
        n = 0;
    }

    try {
        if (n == 0) {
            self->vector.clear();
        } else {
            const auto selfSize = self->vector.size();

            Py_BEGIN_ALLOW_THREADS
                self->vector.resize(selfSize * n);
                for (Py_ssize_t i = 1; i < n; ++i) {
                    simd::simdMemCpyAligned(
                            self->vector.data(),
                            self->vector.data() + selfSize * sizeof(long long) * i,
                            selfSize
                    );
                }
            Py_END_ALLOW_THREADS
        }

        Py_INCREF(pySelf);
        return pySelf;
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static int BigIntArrayList_contains(PyObject *pySelf, PyObject *key) {
    if (!PyLong_Check(key)) {
        return 0;
    }

    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    long long value = PyLong_AsLongLong(key);
    if (PyErr_Occurred()) {
        return -1;
    }

    try {
        if (std::find(self->vector.begin(), self->vector.end(), static_cast<int>(value)) != self->vector.end()) {
            return 1;  // true
        } else {
            return 0;  // false
        }
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }
}

static PyObject *BigIntArrayList_reversed(PyObject *pySelf) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    auto iter = BigIntArrayListIter_create(self, true);
    if (iter == nullptr) return PyErr_NoMemory();
    return reinterpret_cast<PyObject *>(iter);
}

static PyObject *BigIntArrayList_reverse(PyObject *pySelf) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    Py_BEGIN_ALLOW_THREADS
        simd::simdReverseAligned(self->vector.data(), self->vector.size());
    Py_END_ALLOW_THREADS
    Py_RETURN_NONE;
}

static PyObject *BigIntArrayList_clear(PyObject *pySelf) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    self->vector.clear();
    Py_RETURN_NONE;
}

static __forceinline PyObject *BigIntArrayList_eq(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    if (Py_TYPE(pyValue) == &BigIntArrayListType) {
        // fast compare
        auto *value = reinterpret_cast<BigIntArrayList *>(pyValue);
        if (value->vector.size() != self->vector.size())
            Py_RETURN_FALSE;
        if (memcmp(self->vector.data(), value->vector.data(), self->vector.size() * sizeof(long long)) == 0)
            Py_RETURN_TRUE;
        else
            Py_RETURN_FALSE;
    }

    if (!PySequence_Check(pyValue))
        Py_RETURN_FALSE;

    // for others
    Py_ssize_t seq_size = PySequence_Size(pyValue);
    if (seq_size != static_cast<Py_ssize_t>(self->vector.size()))
        Py_RETURN_FALSE;

    for (Py_ssize_t i = 0; i < seq_size; i++) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr) return nullptr;
        long long self_value = self->vector[i];
        long long other_value = PyLong_AsLongLong(item);
        if (PyErr_Occurred()) {  // can't be covert to int
            SAFE_DECREF(item);
            Py_RETURN_FALSE;
        }
        SAFE_DECREF(item);
        if (self_value != other_value)
            Py_RETURN_FALSE;
    }

    Py_RETURN_TRUE;
}

static __forceinline PyObject *BigIntArrayList_ne(PyObject *pySelf, PyObject *pyValue) {
    PyObject *isEq = BigIntArrayList_eq(pySelf, pyValue);
    if (PyObject_IsTrue(isEq)) {
        SAFE_DECREF(isEq);
        Py_RETURN_FALSE;
    } else {
        SAFE_DECREF(isEq);
        Py_RETURN_TRUE;
    }
}

static __forceinline PyObject *BigIntArrayList_lt(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    if (Py_TYPE(pyValue) == &BigIntArrayListType) {
        auto *value = reinterpret_cast<BigIntArrayList *>(pyValue);
        size_t min_size = std::min(self->vector.size(), value->vector.size());

        for (size_t i = 0; i < min_size; i++) {
            if (self->vector[i] != value->vector[i]) {
                Py_RETURN_BOOL(self->vector[i] < value->vector[i])
            }
        }
        Py_RETURN_BOOL(self->vector.size() < value->vector.size())
    }

    if (!PySequence_Check(pyValue))
        Py_RETURN_FALSE;

    Py_ssize_t seq_size = PySequence_Size(pyValue);
    Py_ssize_t min_size = std::min(static_cast<Py_ssize_t>(self->vector.size()), seq_size);

    for (Py_ssize_t i = 0; i < min_size; i++) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr) return nullptr;
        long long self_value = self->vector[i];
        long long other_value = PyLong_AsLongLong(item);
        SAFE_DECREF(item);
        if (self_value != other_value) Py_RETURN_BOOL(self_value < other_value)
    }
    Py_RETURN_BOOL(self->vector.size() < static_cast<size_t>(seq_size))
}

static __forceinline PyObject *BigIntArrayList_le(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    if (Py_TYPE(pyValue) == &BigIntArrayListType) {
        auto *value = reinterpret_cast<BigIntArrayList *>(pyValue);
        size_t min_size = std::min(self->vector.size(), value->vector.size());

        for (size_t i = 0; i < min_size; i++) {
            if (self->vector[i] != value->vector[i]) {
                Py_RETURN_BOOL(self->vector[i] <= value->vector[i])
            }
        }
        Py_RETURN_BOOL(self->vector.size() <= value->vector.size())
    }

    if (!PySequence_Check(pyValue))
        Py_RETURN_FALSE;

    Py_ssize_t seq_size = PySequence_Size(pyValue);
    Py_ssize_t min_size = std::min(static_cast<Py_ssize_t>(self->vector.size()), seq_size);

    for (Py_ssize_t i = 0; i < min_size; i++) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr) return nullptr;
        long long self_value = self->vector[i];
        long long other_value = PyLong_AsLongLong(item);
        Py_DECREF(item);
        if (self_value != other_value) Py_RETURN_BOOL(self_value <= other_value)
    }
    Py_RETURN_BOOL(self->vector.size() <= static_cast<size_t>(seq_size))
}

static __forceinline PyObject *BigIntArrayList_gt(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    if (Py_TYPE(pyValue) == &BigIntArrayListType) {
        auto *value = reinterpret_cast<BigIntArrayList *>(pyValue);
        size_t min_size = std::min(self->vector.size(), value->vector.size());

        for (size_t i = 0; i < min_size; i++) {
            if (self->vector[i] != value->vector[i]) {
                Py_RETURN_BOOL(self->vector[i] > value->vector[i])
            }
        }
        Py_RETURN_BOOL(self->vector.size() > value->vector.size())
    }

    if (!PySequence_Check(pyValue))
        Py_RETURN_FALSE;

    Py_ssize_t seq_size = PySequence_Size(pyValue);
    Py_ssize_t min_size = std::min(static_cast<Py_ssize_t>(self->vector.size()), seq_size);

    for (Py_ssize_t i = 0; i < min_size; i++) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr) return nullptr;
        long long self_value = self->vector[i];
        long long other_value = PyLong_AsLongLong(item);
        Py_DECREF(item);
        if (self_value != other_value) Py_RETURN_BOOL(self_value > other_value)
    }
    Py_RETURN_BOOL(self->vector.size() > static_cast<size_t>(seq_size))
}

static __forceinline PyObject *BigIntArrayList_ge(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    if (Py_TYPE(pyValue) == &BigIntArrayListType) {
        auto *value = reinterpret_cast<BigIntArrayList *>(pyValue);
        size_t min_size = std::min(self->vector.size(), value->vector.size());

        for (size_t i = 0; i < min_size; i++) {
            if (self->vector[i] != value->vector[i]) {
                Py_RETURN_BOOL(self->vector[i] >= value->vector[i])
            }
        }
        Py_RETURN_BOOL(self->vector.size() >= value->vector.size())
    }

    if (!PySequence_Check(pyValue))
        Py_RETURN_FALSE;

    Py_ssize_t seq_size = PySequence_Size(pyValue);
    Py_ssize_t min_size = std::min(static_cast<Py_ssize_t>(self->vector.size()), seq_size);

    for (Py_ssize_t i = 0; i < min_size; i++) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr) return nullptr;
        long long self_value = self->vector[i];
        long long other_value = PyLong_AsLongLong(item);
        Py_DECREF(item);
        if (self_value != other_value) Py_RETURN_BOOL(self_value >= other_value)
    }
    Py_RETURN_BOOL(self->vector.size() >= static_cast<size_t>(seq_size))
}

static PyObject *BigIntArrayList_compare(PyObject *pySelf, PyObject *pyValue, int op) {
    switch (op) {
        case Py_EQ:  // ==
            return BigIntArrayList_eq(pySelf, pyValue);
        case Py_NE:  // !=
            return BigIntArrayList_ne(pySelf, pyValue);
        case Py_LT:  // <
            return BigIntArrayList_lt(pySelf, pyValue);
        case Py_LE:  // <=
            return BigIntArrayList_le(pySelf, pyValue);
        case Py_GT:  // >
            return BigIntArrayList_gt(pySelf, pyValue);
        case Py_GE:  // >=
            return BigIntArrayList_ge(pySelf, pyValue);
        default:
            PyErr_SetString(PyExc_AssertionError, "Invalid comparison operation.");
            return nullptr;
    }
}

static PyObject *BigIntArrayList_class_getitem(PyObject *cls, PyObject *item) {
    return Py_GenericAlias(cls, item);
}

static __forceinline PyObject *BigIntArrayList_repr(PyObject *pySelf) {
    auto *self = reinterpret_cast<BigIntArrayList *>(pySelf);

    const auto &vec = self->vector;

    if (vec.empty()) {
        return PyUnicode_FromString("[]");
    }

    size_t size = vec.size();
    auto str = std::string("[");
    str.reserve(size * 4);

    char buffer[32];

    const size_t lastIdx = size - 1;
    for (size_t i = 0; i < lastIdx; ++i) {
        // to string
        int len = snprintf(buffer, sizeof(buffer), "%lld", vec[i]);
        str.append(buffer, len);
        str += ", ";
    }

    int len = snprintf(buffer, sizeof(buffer), "%lld", vec[lastIdx]);
    str.append(buffer, len);

    str += "]";

    return PyUnicode_FromString(str.c_str());
}

static PyObject *BigIntArrayList_str(PyObject *pySelf) {
    return BigIntArrayList_repr(pySelf);
}

static PyMethodDef BigIntArrayList_methods[] = {
        {"from_range", (PyCFunction) BigIntArrayList_from_range, METH_VARARGS | METH_STATIC},
        {"resize", (PyCFunction) BigIntArrayList_resize, METH_O},
        {"tolist", (PyCFunction) BigIntArrayList_to_list, METH_NOARGS},
        {"copy", (PyCFunction) BigIntArrayList_copy, METH_NOARGS},
        {"append", (PyCFunction) BigIntArrayList_append, METH_O},
        {"extend", (PyCFunction) BigIntArrayList_extend, METH_FASTCALL},
        {"pop", (PyCFunction) BigIntArrayList_pop, METH_FASTCALL},
        {"index", (PyCFunction) BigIntArrayList_index, METH_VARARGS},
        {"count", (PyCFunction) BigIntArrayList_count, METH_O},
        {"insert", (PyCFunction) BigIntArrayList_insert, METH_VARARGS},
        {"remove", (PyCFunction) BigIntArrayList_remove, METH_O},
        {"sort", (PyCFunction) BigIntArrayList_sort, METH_VARARGS | METH_KEYWORDS},
        {"reverse", (PyCFunction) BigIntArrayList_reverse, METH_NOARGS},
        {"clear", (PyCFunction) BigIntArrayList_clear, METH_NOARGS},
        {"__rmul__", (PyCFunction) BigIntArrayList_rmul, METH_O},
        {"__reversed__", (PyCFunction) BigIntArrayList_reversed, METH_NOARGS},
        {"__class_getitem__", (PyCFunction) BigIntArrayList_class_getitem, METH_O | METH_CLASS},
        {nullptr}
};

static struct PyModuleDef BigIntArrayList_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.BigIntArrayList",
        "An BigIntArrayList_module that creates an BigIntArrayList",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

static PySequenceMethods BigIntArrayList_asSequence = {
        BigIntArrayList_len,
        BigIntArrayList_add,
        BigIntArrayList_mul,
        BigIntArrayList_getitem,
        nullptr,
        BigIntArrayList_setitem,
        nullptr,
        BigIntArrayList_contains,
        BigIntArrayList_iadd,
        BigIntArrayList_imul
};

static PyMappingMethods BigIntArrayList_asMapping = {
        BigIntArrayList_len,
        BigIntArrayList_getitem_slice,
        BigIntArrayList_setitem_slice
};

void initializeBigIntArrayListType(PyTypeObject &type) {
    type.tp_name = "BigIntArrayList";
    type.tp_basicsize = sizeof(BigIntArrayList);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_as_sequence = &BigIntArrayList_asSequence;
    type.tp_as_mapping = &BigIntArrayList_asMapping;
    type.tp_iter = BigIntArrayList_iter;
    type.tp_methods = BigIntArrayList_methods;
    type.tp_init = (initproc) BigIntArrayList_init;
    type.tp_new = PyType_GenericNew;
    type.tp_dealloc = (destructor) BigIntArrayList_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
    type.tp_hash = PyObject_HashNotImplemented;
    type.tp_richcompare = BigIntArrayList_compare;
    type.tp_repr = BigIntArrayList_repr;
    type.tp_str = BigIntArrayList_str;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_BigIntArrayList() {
    initializeBigIntArrayListType(BigIntArrayListType);

    PyObject *object = PyModule_Create(&BigIntArrayList_module);
    if (object == nullptr)
        return nullptr;

    Py_INCREF(&BigIntArrayListType);
    if (PyModule_AddObject(object, "BigIntArrayList", (PyObject *) &BigIntArrayListType) < 0) {
        Py_DECREF(&BigIntArrayListType);
        Py_DECREF(object);
        return nullptr;
    }

    return object;
}
#pragma clang diagnostic pop

}