//
// Created by xia__mc on 2024/11/7.
//

#include "IntArrayList.h"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "utils/PythonUtils.h"
#include "utils/include/TimSort.h"
#include "utils/simd/BitonicSort.h"
#include "utils/simd/SIMDUtils.h"
#include "utils/memory/AlignedAllocator.h"
#include "ints/IntArrayListIter.h"
#include "utils/include/CPythonSort.h"

extern "C" {

static PyTypeObject IntArrayListType = {
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

static int IntArrayList_init(IntArrayList *self, PyObject *args, PyObject *kwargs) {
    new(&self->vector) std::vector<int, AlignedAllocator<int, 64>>();

    PyObject *pyIterable = nullptr;
    Py_ssize_t pySize = -1;

    parseArgs(args, kwargs, pyIterable, pySize);

    // init vector
    try {
        if (pySize > 0) {
            self->vector.reserve(static_cast<size_t>(pySize));
        }

        if (pyIterable != nullptr) {
            if (Py_TYPE(pyIterable) == &IntArrayListType) {  // IntArrayList is a final class
                auto *iter = reinterpret_cast<IntArrayList *>(pyIterable);
                self->vector = iter->vector;
                return 0;
            }

            if (PyList_Check(pyIterable) || PyTuple_Check(pyIterable)) {  // fast operation
                auto fastKeys = PySequence_Fast(pyIterable, "Shouldn't be happen (IntArrayList).");
                if (fastKeys == nullptr) {
                    return -1;
                }

                const auto size = PySequence_Fast_GET_SIZE(fastKeys);
                auto items = PySequence_Fast_ITEMS(fastKeys);
                for (Py_ssize_t i = 0; i < size; ++i) {
                    int value = PyLong_AsLong(items[i]);
                    if (PyErr_Occurred()) {
                        SAFE_DECREF(fastKeys);
                        PyErr_SetString(PyExc_RuntimeError, "Failed to convert item to C int during iteration.");
                        return -1;
                    }
                    self->vector.push_back(value);
                }
                SAFE_DECREF(fastKeys);
                return 0;
            }

            PyObject *iter = PyObject_GetIter(pyIterable);
            if (iter == nullptr) {
                PyErr_SetString(PyExc_TypeError, "Arg '__iterable' is not iterable.");
                return -1;
            }

            PyObject *item;
            while ((item = PyIter_Next(iter)) != nullptr) {
                int value = PyLong_AsLong(item);
                if (PyErr_Occurred()) {
                    SAFE_DECREF(iter);
                    SAFE_DECREF(item);
                    PyErr_SetString(PyExc_RuntimeError, "Failed to convert item to C int during iteration.");
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

static PyObject *IntArrayList_from_range([[maybe_unused]] PyObject *cls, PyObject *args) {
    Py_ssize_t start, stop, step;

    if (!PyParse_EvalRange(args, start, stop, step)) {
        return nullptr;
    }

    auto *list = Py_CreateObj<IntArrayList>(IntArrayListType);
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
            list->vector[idx] = static_cast<int>(current);
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

static PyObject *IntArrayList_resize(PyObject *pySelf, PyObject *pySize) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

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

static PyObject *IntArrayList_to_list(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    const auto size = static_cast<Py_ssize_t>(self->vector.size());
    PyObject *result = PyList_New(size);
    if (result == nullptr) return PyErr_NoMemory();

    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject *item = PyFast_FromInt(self->vector[i]);
        if (item == nullptr) {
            SAFE_DECREF(result);
            return nullptr;
        }

        PyList_SET_ITEM(result, i, item);  // PyList_SET_ITEM handle this ref
    }

    return result;
}

static PyObject *IntArrayList_copy(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    auto *copy = Py_CreateObj<IntArrayList>(IntArrayListType);
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

    int value = PyLong_AsLong(object);
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

static PyObject *IntArrayList_extend(PyObject *pySelf, PyObject *const *args, const Py_ssize_t nargs) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    // FASTCALL ensure args != nullptr
    if (nargs != 1) {
        PyErr_SetString(PyExc_TypeError, "extend() takes exactly one argument");
        return nullptr;
    }

    PyObject *iterable = args[0];

    // fast extend
    if (Py_TYPE(iterable) == &IntArrayListType) {
        auto *iter = reinterpret_cast<IntArrayList *>(iterable);
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
        int value = PyLong_AsLong(item);
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


static PyObject *IntArrayList_pop(PyObject *pySelf, PyObject *const *args, const Py_ssize_t nargs) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    if (self->vector.empty()) {
        PyErr_SetString(PyExc_IndexError, "pop from empty list");
        return nullptr;
    }

    const auto vecSize = static_cast<Py_ssize_t>(self->vector.size());
    Py_ssize_t index = vecSize - 1;

    int popped;
    switch (nargs) {
        case 0:
            popped = self->vector.back();
            self->vector.pop_back();

            return PyFast_FromInt(popped);
        case 1:
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

            popped = self->vector[static_cast<size_t>(index)];
            self->vector.erase(self->vector.begin() + index);

            return PyFast_FromInt(popped);
        default:
            PyErr_SetString(PyExc_TypeError, "pop() takes at most 1 argument");
            return nullptr;
    }
    Py_UNREACHABLE();
}

static PyObject *IntArrayList_index(PyObject *pySelf, PyObject *args) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    int value;
    Py_ssize_t start = 0;
    auto stop = static_cast<Py_ssize_t>(self->vector.size());

    if (!PyArg_ParseTuple(args, "i|nn", &value, &start, &stop)) {
        return nullptr;
    }

    if (start < 0) {
        start += static_cast<Py_ssize_t>(self->vector.size());
    }
    if (stop < 0) {
        stop += static_cast<Py_ssize_t>(self->vector.size());
    }

    if (start < 0) {
        start = 0;
    }
    if (stop > static_cast<Py_ssize_t>(self->vector.size())) {
        stop = static_cast<Py_ssize_t>(self->vector.size());
    }

    if (start > stop) {
        PyErr_SetString(PyExc_ValueError, "start index cannot be greater than stop index.");
        return nullptr;
    }

    auto it = std::find(self->vector.begin() + start, self->vector.begin() + stop, value);

    if (it == self->vector.begin() + stop) {
        PyErr_SetString(PyExc_ValueError, "Value is not in list.");
        return nullptr;
    }

    Py_ssize_t index = std::distance(self->vector.begin(), it);
    return PyLong_FromSsize_t(index);
}

static PyObject *IntArrayList_count(PyObject *pySelf, PyObject *object) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    int value = PyLong_AsLong(object);
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

static PyObject *IntArrayList_insert(PyObject *pySelf, PyObject *args) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    Py_ssize_t index;
    int value;

    if (!PyArg_ParseTuple(args, "ni", &index, &value)) {
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

static PyObject *IntArrayList_remove(PyObject *pySelf, PyObject *object) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    int value = PyLong_AsLong(object);
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

static PyObject *IntArrayList_sort(PyObject *pySelf, PyObject *args, PyObject *kwargs) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    PyObject *keyFunc = nullptr;
    int reverseInt = 0;  // default: false
    static constexpr const char *kwlist[] = {"key", "reverse", nullptr};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|Oi", const_cast<char **>(kwlist), &keyFunc, &reverseInt)) {
        return nullptr;
    }

    const bool reverse = reverseInt == 1;

    // do sort
    try {
        if (keyFunc == nullptr || keyFunc == Py_None) {
            // simd sort with auto-fallback
            Py_BEGIN_ALLOW_THREADS
                simd::simdsort(self->vector, reverse);
            Py_END_ALLOW_THREADS
        } else {
            // sort with key function, costs extra memory
            const auto vecSize = self->vector.size();
            auto vecData = self->vector.data();
            auto **pyData = static_cast<PyObject **>(PyMem_Malloc(sizeof(PyObject *) * vecSize));
            if (pyData == nullptr) {
                PyErr_NoMemory();
                return nullptr;
            }

            for (size_t i = 0; i < vecSize; ++i) {
                pyData[i] = PyFast_FromInt(vecData[i]);
            }

            CPython_sort(pyData, static_cast<Py_ssize_t>(vecSize), keyFunc, reverseInt);

            for (size_t i = 0; i < vecSize; ++i) {
                vecData[i] = PyFast_AsInt(pyData[i]);
                Py_DECREF(pyData[i]);
            }

            PyMem_FREE(pyData);
        }
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    Py_RETURN_NONE;
}

static Py_ssize_t IntArrayList_len(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    return static_cast<Py_ssize_t>(self->vector.size());
}


static PyObject *IntArrayList_iter(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    auto iter = IntArrayListIter_create(self);
    if (iter == nullptr) return PyErr_NoMemory();
    return reinterpret_cast<PyObject *>(iter);
}

static PyObject *IntArrayList_getitem(PyObject *pySelf, Py_ssize_t pyIndex) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    auto size = static_cast<Py_ssize_t>(self->vector.size());

    if (pyIndex < 0) {
        pyIndex = size + pyIndex;
    }

    if (pyIndex < 0 || pyIndex >= size) {
        PyErr_SetString(PyExc_IndexError, "index out of range.");
        return nullptr;
    }

    return PyFast_FromInt(self->vector[static_cast<size_t>(pyIndex)]);
}

static PyObject *IntArrayList_getitem_slice(PyObject *pySelf, PyObject *slice) {
    if (PyIndex_Check(slice)) {
        Py_ssize_t pyIndex = PyNumber_AsSsize_t(slice, PyExc_IndexError);
        if (pyIndex == -1 && PyErr_Occurred()) {
            return nullptr;
        }
        return IntArrayList_getitem(pySelf, pyIndex);
    }

    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

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
        PyObject *item = PyFast_FromInt(self->vector[static_cast<size_t>(index)]);
        if (item == nullptr) {
            SAFE_DECREF(result);
            return nullptr;
        }
        PyList_SET_ITEM(result, i, item);
    }
    return result;
}

static int IntArrayList_setitem(PyObject *pySelf, Py_ssize_t pyIndex, PyObject *pyValue) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

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
            int value = PyLong_AsLong(pyValue);
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


static int IntArrayList_setitem_slice(PyObject *pySelf, PyObject *slice, PyObject *value) {
    if (PyIndex_Check(slice)) {
        Py_ssize_t index = PyNumber_AsSsize_t(slice, PyExc_IndexError);
        if (index == -1 && PyErr_Occurred()) {
            return -1;
        }
        return IntArrayList_setitem(pySelf, index, value);
    }

    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    Py_ssize_t start, stop, step, sliceLength;
    if (PySlice_Unpack(slice, &start, &stop, &step) < 0) {
        return -1;
    }

    sliceLength = PySlice_AdjustIndices(static_cast<Py_ssize_t>(self->vector.size()), &start, &stop, step);

    if (!PySequence_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "can only assign an iterable");
        return -1;
    }

    Py_ssize_t newLength = PySequence_Size(value);

    if (step != 1) {
        PyErr_SetString(PyExc_NotImplementedError, "step must be 1 for slice assignment");
        return -1;
    }

    PyObject *item = nullptr;
    try {
        if (newLength == sliceLength) {  // only change elements
            for (Py_ssize_t i = 0; i < sliceLength; i++) {
                item = PySequence_GetItem(value, i);
                if (item == nullptr) {
                    return -1;
                }

                self->vector[static_cast<size_t>(start + i)] = PyLong_AsLong(item);
                SAFE_DECREF(item);
                if (PyErr_Occurred()) {
                    return -1;
                }
            }
        } else if (newLength < sliceLength) {  // less elements
            for (Py_ssize_t i = 0; i < newLength; i++) {
                item = PySequence_GetItem(value, i);
                if (item == nullptr) {
                    return -1;
                }

                self->vector[static_cast<size_t>(start + i)] = PyLong_AsLong(item);
                SAFE_DECREF(item);
                if (PyErr_Occurred()) {
                    return -1;
                }
            }

            self->vector.erase(
                self->vector.begin() + start + newLength,
                self->vector.begin() + stop
            );
        } else {  // more elements
            for (Py_ssize_t i = 0; i < sliceLength; i++) {
                item = PySequence_GetItem(value, i);
                if (item == nullptr) {
                    return -1;
                }

                self->vector[static_cast<size_t>(start + i)] = PyLong_AsLong(item);
                SAFE_DECREF(item);
                if (PyErr_Occurred()) {
                    return -1;
                }
            }

            for (Py_ssize_t i = sliceLength; i < newLength; i++) {
                item = PySequence_GetItem(value, i);
                if (item == nullptr) {
                    return -1;
                }

                self->vector.insert(
                    self->vector.begin() + start + i,
                    PyLong_AsLong(item)
                );
                SAFE_DECREF(item);
                if (PyErr_Occurred()) {
                    return -1;
                }
            }
        }
    } catch (const std::exception &e) {
        SAFE_DECREF(item);
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }

    return 0;
}

static PyObject *IntArrayList_add(PyObject *pySelf, PyObject *pyValue) {
    if (Py_TYPE(pyValue) == &IntArrayListType) {
        // fast add -> IntArrayList
        auto *value = reinterpret_cast<IntArrayList *>(pyValue);

        auto *result = Py_CreateObj<IntArrayList>(IntArrayListType, pySelf);
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
    PyObject *selfIntList = IntArrayList_to_list(pySelf);
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

static PyObject *IntArrayList_iadd(PyObject *pySelf, PyObject *iterable) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    // fast extend
    if (Py_TYPE(iterable) == &IntArrayListType) {
        auto *iter = reinterpret_cast<IntArrayList *>(iterable);
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
        int value = PyLong_AsLong(item);
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


static PyObject *IntArrayList_mul(PyObject *pySelf, Py_ssize_t n) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    if (n < 0) {
        n = 0;
    }

    auto *result = Py_CreateObj<IntArrayList>(IntArrayListType);
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

static PyObject *IntArrayList_rmul(PyObject *pySelf, PyObject *pyValue) {
    if (PyLong_Check(pyValue)) {
        Py_ssize_t n = PyLong_AsSsize_t(pyValue);
        if (PyErr_Occurred()) {
            return nullptr;
        }

        return IntArrayList_mul(pySelf, n);
    }

    PyErr_SetString(PyExc_TypeError, "Expected an integer on the left-hand side of *");
    return nullptr;
}

static PyObject *IntArrayList_imul(PyObject *pySelf, Py_ssize_t n) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

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
                            self->vector.data() + selfSize * sizeof(int) * i,
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

static int IntArrayList_contains(PyObject *pySelf, PyObject *key) {
    if (!PyLong_Check(key)) {
        return 0;
    }

    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    long value = PyLong_AsLong(key);
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

static PyObject *IntArrayList_reversed(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    auto iter = IntArrayListIter_create(self, true);
    if (iter == nullptr) return PyErr_NoMemory();
    return reinterpret_cast<PyObject *>(iter);
}

static PyObject *IntArrayList_reverse(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    Py_BEGIN_ALLOW_THREADS
        simd::simdReverse(self->vector.data(), self->vector.size());
    Py_END_ALLOW_THREADS
    Py_RETURN_NONE;
}

static PyObject *IntArrayList_clear(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    self->vector.clear();
    Py_RETURN_NONE;
}

static __forceinline PyObject *IntArrayList_eq(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    if (Py_TYPE(pyValue) == &IntArrayListType) {
        // fast compare
        auto *value = reinterpret_cast<IntArrayList *>(pyValue);
        if (value->vector.size() != self->vector.size())
            Py_RETURN_FALSE;
        if (memcmp(self->vector.data(), value->vector.data(), self->vector.size() * sizeof(int)) == 0)
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
        int self_value = self->vector[i];
        int other_value = PyLong_AsLong(item);
        if (PyErr_Occurred()) {  // can't be converted to int
            SAFE_DECREF(item);
            Py_RETURN_FALSE;
        }
        SAFE_DECREF(item);
        if (self_value != other_value)
            Py_RETURN_FALSE;
    }

    Py_RETURN_TRUE;
}

static __forceinline PyObject *IntArrayList_ne(PyObject *pySelf, PyObject *pyValue) {
    PyObject *isEq = IntArrayList_eq(pySelf, pyValue);
    if (PyObject_IsTrue(isEq)) {
        SAFE_DECREF(isEq);
        Py_RETURN_FALSE;
    } else {
        SAFE_DECREF(isEq);
        Py_RETURN_TRUE;
    }
}

static __forceinline PyObject *IntArrayList_lt(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    if (Py_TYPE(pyValue) == &IntArrayListType) {
        auto *value = reinterpret_cast<IntArrayList *>(pyValue);
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
        int self_value = self->vector[i];
        int other_value = PyLong_AsLong(item);
        SAFE_DECREF(item);
        if (self_value != other_value) Py_RETURN_BOOL(self_value < other_value)
    }
    Py_RETURN_BOOL(self->vector.size() < static_cast<size_t>(seq_size))
}

static __forceinline PyObject *IntArrayList_le(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    if (Py_TYPE(pyValue) == &IntArrayListType) {
        auto *value = reinterpret_cast<IntArrayList *>(pyValue);
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
        int self_value = self->vector[i];
        int other_value = PyLong_AsLong(item);
        Py_DECREF(item);
        if (self_value != other_value) Py_RETURN_BOOL(self_value <= other_value)
    }
    Py_RETURN_BOOL(self->vector.size() <= static_cast<size_t>(seq_size))
}

static __forceinline PyObject *IntArrayList_gt(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    if (Py_TYPE(pyValue) == &IntArrayListType) {
        auto *value = reinterpret_cast<IntArrayList *>(pyValue);
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
        int self_value = self->vector[i];
        int other_value = PyLong_AsLong(item);
        Py_DECREF(item);
        if (self_value != other_value) Py_RETURN_BOOL(self_value > other_value)
    }
    Py_RETURN_BOOL(self->vector.size() > static_cast<size_t>(seq_size))
}

static __forceinline PyObject *IntArrayList_ge(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    if (Py_TYPE(pyValue) == &IntArrayListType) {
        auto *value = reinterpret_cast<IntArrayList *>(pyValue);
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
        int self_value = self->vector[i];
        int other_value = PyLong_AsLong(item);
        Py_DECREF(item);
        if (self_value != other_value) Py_RETURN_BOOL(self_value >= other_value)
    }
    Py_RETURN_BOOL(self->vector.size() >= static_cast<size_t>(seq_size))
}

static PyObject *IntArrayList_compare(PyObject *pySelf, PyObject *pyValue, int op) {
    switch (op) {
        case Py_EQ:  // ==
            return IntArrayList_eq(pySelf, pyValue);
        case Py_NE:  // !=
            return IntArrayList_ne(pySelf, pyValue);
        case Py_LT:  // <
            return IntArrayList_lt(pySelf, pyValue);
        case Py_LE:  // <=
            return IntArrayList_le(pySelf, pyValue);
        case Py_GT:  // >
            return IntArrayList_gt(pySelf, pyValue);
        case Py_GE:  // >=
            return IntArrayList_ge(pySelf, pyValue);
        default:
            PyErr_SetString(PyExc_AssertionError, "Invalid comparison operation.");
            return nullptr;
    }
}

#ifdef IS_PYTHON_39_OR_LATER
static PyObject *IntArrayList_class_getitem(PyObject *cls, PyObject *item) {
    return Py_GenericAlias(cls, item);
}
#endif

static __forceinline PyObject *IntArrayList_repr(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

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
        int len = snprintf(buffer, sizeof(buffer), "%d", vec[i]);
        str.append(buffer, len);
        str += ", ";
    }

    int len = snprintf(buffer, sizeof(buffer), "%d", vec[lastIdx]);
    str.append(buffer, len);

    str += "]";

    return PyUnicode_FromString(str.c_str());
}

static PyObject *IntArrayList_str(PyObject *pySelf) {
    return IntArrayList_repr(pySelf);
}

static int IntArrayList_get_buffer(PyObject *pySelf, Py_buffer *view, [[maybe_unused]] int flags) {
    if (view == nullptr) return -1;
    auto *self = reinterpret_cast<IntArrayList *>(pySelf);

    self->shape = static_cast<Py_ssize_t>(self->vector.size());
    view->format = const_cast<char *>("i");
    view->buf = self->vector.data();
    view->len = static_cast<Py_ssize_t>(self->vector.size() * sizeof(int));
    view->itemsize = sizeof(int);
    view->readonly = 0;
    view->ndim = 1;
    view->shape = &self->shape;
    view->strides = nullptr;
    view->suboffsets = nullptr;
    view->internal = nullptr;

    return 0;
}

static PyMethodDef IntArrayList_methods[] = {
        {"from_range", (PyCFunction) IntArrayList_from_range, METH_VARARGS | METH_STATIC},
        {"resize", (PyCFunction) IntArrayList_resize, METH_O},
        {"to_list", (PyCFunction) IntArrayList_to_list, METH_NOARGS},
        {"copy", (PyCFunction) IntArrayList_copy, METH_NOARGS},
        {"append", (PyCFunction) IntArrayList_append, METH_O},
        {"extend", (PyCFunction) IntArrayList_extend, METH_FASTCALL},
        {"pop", (PyCFunction) IntArrayList_pop, METH_FASTCALL},
        {"index", (PyCFunction) IntArrayList_index, METH_VARARGS},
        {"count", (PyCFunction) IntArrayList_count, METH_O},
        {"insert", (PyCFunction) IntArrayList_insert, METH_VARARGS},
        {"remove", (PyCFunction) IntArrayList_remove, METH_O},
        {"sort", (PyCFunction) IntArrayList_sort, METH_VARARGS | METH_KEYWORDS},
        {"reverse", (PyCFunction) IntArrayList_reverse, METH_NOARGS},
        {"clear", (PyCFunction) IntArrayList_clear, METH_NOARGS},
        {"__rmul__", (PyCFunction) IntArrayList_rmul, METH_O},
        {"__reversed__", (PyCFunction) IntArrayList_reversed, METH_NOARGS},
#ifdef IS_PYTHON_39_OR_LATER
        {"__class_getitem__", (PyCFunction) IntArrayList_class_getitem, METH_O | METH_CLASS},
#endif
        {nullptr}
};

static struct PyModuleDef IntArrayList_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.IntArrayList",
        "An IntArrayList_module that creates an IntArrayList",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

static PySequenceMethods IntArrayList_asSequence = {
        IntArrayList_len,
        IntArrayList_add,
        IntArrayList_mul,
        IntArrayList_getitem,
        nullptr,
        IntArrayList_setitem,
        nullptr,
        IntArrayList_contains,
        IntArrayList_iadd,
        IntArrayList_imul
};

static PyMappingMethods IntArrayList_asMapping = {
        IntArrayList_len,
        IntArrayList_getitem_slice,
        IntArrayList_setitem_slice
};

static PyBufferProcs IntArrayList_asBuffer = {
        IntArrayList_get_buffer,
        nullptr
};

void initializeIntArrayListType(PyTypeObject &type) {
    type.tp_name = "IntArrayList";
    type.tp_basicsize = sizeof(IntArrayList);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_as_sequence = &IntArrayList_asSequence;
    type.tp_as_mapping = &IntArrayList_asMapping;
    type.tp_iter = IntArrayList_iter;
    type.tp_methods = IntArrayList_methods;
    type.tp_init = (initproc) IntArrayList_init;
    type.tp_new = PyType_GenericNew;
    type.tp_dealloc = (destructor) IntArrayList_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
    type.tp_hash = PyObject_HashNotImplemented;
    type.tp_richcompare = IntArrayList_compare;
    type.tp_repr = IntArrayList_repr;
    type.tp_str = IntArrayList_str;
    type.tp_as_buffer = &IntArrayList_asBuffer;
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
