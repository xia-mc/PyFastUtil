//
// Created by xia__mc on 2024/11/24.
//

#include "IntLinkedList.h"
#include <list>
#include <algorithm>
#include <stdexcept>
#include "utils/PythonUtils.h"
#include "utils/include/TimSort.h"
#include "utils/simd/BitonicSort.h"
#include "utils/simd/Utils.h"
#include "utils/memory/AlignedAllocator.h"
#include "ints/IntLinkedListIter.h"
#include "utils/include/CPythonSort.h"
#include "utils/Utils.h"

extern "C" {
static PyTypeObject IntLinkedListType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
};

static int IntLinkedList_init(IntLinkedList *self, PyObject *args, PyObject *kwargs) {
    new(&self->list) std::list<PyObject *>();
    self->modCount = 0;

    PyObject *pyIterable = nullptr;

    static constexpr const char *kwlist[] = {"iterable", nullptr};

    // parse args
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O", const_cast<char **>(kwlist), &pyIterable)) {
        return -1;
    }

    // init list
    try {
        if (pyIterable != nullptr) {
            if (Py_TYPE(pyIterable) == &IntLinkedListType) {  // IntLinkedList is a final class
                auto *iter = reinterpret_cast<IntLinkedList *>(pyIterable);
                self->list = iter->list;
                return 0;
            }

            if (PyList_Check(pyIterable) || PyTuple_Check(pyIterable)) {  // fast operation
                auto fastKeys = PySequence_Fast(pyIterable, "Shouldn't be happen (IntLinkedList).");
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
                    self->list.push_back(value);
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
                self->list.push_back(value);
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

static void IntLinkedList_dealloc(IntLinkedList *self) {
    self->list.~list();
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *IntLinkedList_from_range([[maybe_unused]] PyObject *cls, PyObject *args) {
    Py_ssize_t start, stop, step;

    if (!PyParse_EvalRange(args, start, stop, step)) {
        return nullptr;
    }

    auto *list = Py_CreateObj<IntLinkedList>(IntLinkedListType);
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

        Py_ssize_t current = start;
        for (Py_ssize_t idx = 0; idx < elements; ++idx) {
            list->list.push_back(static_cast<int>(current));
            current += step;
        }

    } catch (const std::exception &e) {
        list->list.~list();
        PyObject_Del(list);
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    return reinterpret_cast<PyObject *>(list);
}

static PyObject *IntLinkedList_to_list(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    const auto size = static_cast<Py_ssize_t>(self->list.size());
    PyObject *result = PyList_New(size);
    if (result == nullptr) return PyErr_NoMemory();

    auto iter = self->list.begin();
    for (Py_ssize_t i = 0; i < size; ++i, ++iter) {
        PyObject *item = PyLong_FromLong(*iter);

        PyList_SET_ITEM(result, i, item);
        Py_INCREF(item);
    }

    return result;
}

static PyObject *IntLinkedList_copy(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    auto *copy = Py_CreateObj<IntLinkedList>(IntLinkedListType);
    if (copy == nullptr) return PyErr_NoMemory();

    try {
        copy->list = self->list;
    } catch (const std::exception &e) {
        PyObject_Del(copy);
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    return reinterpret_cast<PyObject *>(copy);
}

static PyObject *IntLinkedList_append(PyObject *pySelf, PyObject *object) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    PyObject *value = object;
    if (PyErr_Occurred()) {
        return nullptr;
    }

    try {
        self->list.push_back(PyLong_AsLong(value));
        self->modCount++;
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    Py_RETURN_NONE;
}

static PyObject *IntLinkedList_extend(PyObject *pySelf, PyObject *const *args, const Py_ssize_t nargs) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    // FASTCALL ensure args != nullptr
    if (nargs != 1) {
        PyErr_SetString(PyExc_TypeError, "extend() takes exactly one argument");
        return nullptr;
    }

    PyObject *iterable = args[0];

    // fast extend
    if (Py_TYPE(iterable) == &IntLinkedListType) {
        auto *iter = reinterpret_cast<IntLinkedList *>(iterable);
        self->list.insert(self->list.end(), iter->list.begin(), iter->list.end());
        self->modCount++;
        Py_RETURN_NONE;
    }

    // python iterable extend
    PyObject *iter = PyObject_GetIter(iterable);
    if (iter == nullptr) {
        return nullptr;
    }

    // do extend
    PyObject *item;
    while ((item = PyIter_Next(iter)) != nullptr) {
        if (PyErr_Occurred()) {
            SAFE_DECREF(iter);
            SAFE_DECREF(item);
            return nullptr;
        }

        self->list.push_back(PyLong_AsLong(item));
        SAFE_DECREF(item);
    }
    self->modCount++;

    if (PyErr_Occurred()) {
        SAFE_DECREF(iter);
        return nullptr;
    }

    SAFE_DECREF(iter);
    Py_RETURN_NONE;
}


static PyObject *IntLinkedList_pop(PyObject *pySelf, PyObject *const *args, const Py_ssize_t nargs) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    if (self->list.empty()) {
        PyErr_SetString(PyExc_IndexError, "pop from empty list");
        return nullptr;
    }

    const auto vecSize = static_cast<Py_ssize_t>(self->list.size());
    Py_ssize_t index;

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
        const auto popped = self->list.back();
        self->list.pop_back();
        self->modCount++;
        return PyLong_FromLong(popped);
    }

    auto popped = at(self->list, index);
    self->list.erase(popped);
    self->modCount++;
    return PyLong_FromLong(*popped);
}

static PyObject *IntLinkedList_index(PyObject *pySelf, PyObject *args) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    int value;
    Py_ssize_t start = 0;
    auto stop = static_cast<Py_ssize_t>(self->list.size());

    if (!PyArg_ParseTuple(args, "i|nn", &value, &start, &stop)) {
        return nullptr;
    }

    if (start < 0) {
        start += static_cast<Py_ssize_t>(self->list.size());
    }
    if (stop < 0) {
        stop += static_cast<Py_ssize_t>(self->list.size());
    }

    if (start < 0) {
        start = 0;
    }
    if (stop > static_cast<Py_ssize_t>(self->list.size())) {
        stop = static_cast<Py_ssize_t>(self->list.size());
    }

    if (start > stop) {
        PyErr_SetString(PyExc_ValueError, "start index cannot be greater than stop index.");
        return nullptr;
    }

    auto stopIter = at(self->list, stop);
    auto it = std::find(at(self->list, start), stopIter, value);

    if (it == stopIter) {
        PyErr_SetString(PyExc_ValueError, "Value is not in list.");
        return nullptr;
    }

    Py_ssize_t index = std::distance(self->list.begin(), it);
    return PyLong_FromSsize_t(index);
}

static PyObject *IntLinkedList_count(PyObject *pySelf, PyObject *object) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    int value = PyLong_AsLong(object);
    if (value == -1 && PyErr_Occurred()) {
        return nullptr;
    }

    try {
        size_t result = std::count(self->list.begin(), self->list.end(), value);

        return PyLong_FromSize_t(result);
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static PyObject *IntLinkedList_insert(PyObject *pySelf, PyObject *args) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    Py_ssize_t index;
    int value;

    if (!PyArg_ParseTuple(args, "ni", &index, &value)) {
        return nullptr;
    }

    // fix index
    const auto vecSize = static_cast<Py_ssize_t>(self->list.size());
    if (index < 0) {
        index = std::max(static_cast<Py_ssize_t>(0), vecSize + index);
    } else if (index > vecSize) {
        index = vecSize;
    }

    // do insert
    try {
        self->list.insert(at(self->list, index), value);
        self->modCount++;
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    Py_RETURN_NONE;
}

static PyObject *IntLinkedList_remove(PyObject *pySelf, PyObject *object) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    int value = PyLong_AsLong(object);
    if (PyErr_Occurred()) {
        return nullptr;
    }

    try {
        auto it = std::find(self->list.begin(), self->list.end(), value);

        if (it != self->list.end()) {
            self->list.erase(it);
            self->modCount++;
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

static PyObject *IntLinkedList_sort(PyObject *pySelf, PyObject *args, PyObject *kwargs) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    PyObject *keyFunc = Py_None;
    int reverseInt = 0;  // default: false
    static constexpr const char *kwlist[] = {"key", "reverse", nullptr};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|Oi", const_cast<char **>(kwlist), &keyFunc, &reverseInt)) {
        return nullptr;
    }

    if (keyFunc == Py_None) {
        if (reverseInt == 1) {
            self->list.sort(std::greater());
        } else {
            self->list.sort();
        }
        self->modCount++;
    } else {
        // costs extra memory
        const auto vecSize = self->list.size();
        auto **pyData = static_cast<PyObject **>(PyMem_Malloc(sizeof(PyObject *) * vecSize));
        if (pyData == nullptr) {
            PyErr_NoMemory();
            return nullptr;
        }

        auto iter = self->list.begin();
        for (size_t i = 0; i < vecSize; ++i, ++iter) {
            pyData[i] = PyLong_FromLong(*iter);
        }

        CPython_sort(pyData,
                     static_cast<Py_ssize_t>(vecSize),
                     keyFunc, reverseInt);

        iter = self->list.begin();
        for (size_t i = 0; i < vecSize; ++i, ++iter) {
            *iter = PyFast_AsInt(pyData[i]);
            Py_DECREF(pyData[i]);
        }

        PyMem_FREE(pyData);
        self->modCount++;
    }

    Py_RETURN_NONE;
}

static Py_ssize_t IntLinkedList_len(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    return static_cast<Py_ssize_t>(self->list.size());
}


static PyObject *IntLinkedList_iter(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    auto iter = IntLinkedListIter_create(self);
    if (iter == nullptr) return PyErr_NoMemory();
    return reinterpret_cast<PyObject *>(iter);
}

static PyObject *IntLinkedList_getitem(PyObject *pySelf, Py_ssize_t pyIndex) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    auto size = static_cast<Py_ssize_t>(self->list.size());

    if (pyIndex < 0) {
        pyIndex = size + pyIndex;
    }

    if (pyIndex < 0 || pyIndex >= size) {
        PyErr_SetString(PyExc_IndexError, "index out of range.");
        return nullptr;
    }

    try {
        PyObject *item = PyLong_FromLong(*at(self->list, static_cast<size_t>(pyIndex)));
        Py_INCREF(item);
        return item;
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static PyObject *IntLinkedList_getitem_slice(PyObject *pySelf, PyObject *slice) {
    if (PyIndex_Check(slice)) {
        Py_ssize_t pyIndex = PyNumber_AsSsize_t(slice, PyExc_IndexError);
        if (pyIndex == -1 && PyErr_Occurred()) {
            return nullptr;
        }
        return IntLinkedList_getitem(pySelf, pyIndex);
    }

    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    Py_ssize_t start, stop, step, sliceLength;
    if (PySlice_Unpack(slice, &start, &stop, &step) < 0) {
        return nullptr;
    }

    sliceLength = PySlice_AdjustIndices(static_cast<Py_ssize_t>(self->list.size()), &start, &stop, step);

    PyObject *result = PyList_New(sliceLength);
    if (!result) {
        return nullptr;
    }

    for (Py_ssize_t i = 0; i < sliceLength; i++) {
        Py_ssize_t index = start + i * step;
        PyObject *item = PyLong_FromLong(*at(self->list, static_cast<size_t>(index)));
        Py_INCREF(item);
        if (item == nullptr) {
            SAFE_DECREF(result);
            return nullptr;
        }
        PyList_SET_ITEM(result, i, item);
    }
    return result;
}

static int IntLinkedList_setitem(PyObject *pySelf, Py_ssize_t pyIndex, PyObject *pyValue) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    auto size = static_cast<Py_ssize_t>(self->list.size());

    if (pyIndex < 0) {
        pyIndex = size + pyIndex;
    }
    if (pyIndex < 0 || pyIndex >= size) {
        PyErr_SetString(PyExc_IndexError, "index out of range.");
        return -1;
    }

    try {
        if (pyValue == nullptr) {
            auto iter = at(self->list, static_cast<size_t>(pyIndex));
            self->list.erase(iter);
        } else {
            PyObject *value = pyValue;
            if (PyErr_Occurred()) {
                return -1;
            }

            *at(self->list, static_cast<size_t>(pyIndex)) = PyLong_AsLong(value);
        }
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }
    return 0;
}


static int IntLinkedList_setitem_slice(PyObject *pySelf, PyObject *slice, PyObject *value) {
    if (PyIndex_Check(slice)) {
        Py_ssize_t index = PyNumber_AsSsize_t(slice, PyExc_IndexError);
        if (index == -1 && PyErr_Occurred()) {
            return -1;
        }
        return IntLinkedList_setitem(pySelf, index, value);
    }

    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    Py_ssize_t start, stop, step, sliceLength;
    if (PySlice_Unpack(slice, &start, &stop, &step) < 0) {
        return -1;
    }

    sliceLength = PySlice_AdjustIndices(static_cast<Py_ssize_t>(self->list.size()), &start, &stop, step);

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
                auto iter = at(self->list, static_cast<size_t>(index));
                self->list.erase(iter);
            } else {
                item = PySequence_GetItem(value, i);
                if (item == nullptr) {
                    return -1;
                }

                *at(self->list, static_cast<size_t>(index)) = PyLong_AsLong(item);
                if (PyErr_Occurred()) {
                    SAFE_DECREF(item);
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

static PyObject *IntLinkedList_add(PyObject *pySelf, PyObject *pyValue) {
    if (Py_TYPE(pyValue) == &IntLinkedListType) {
        // fast add -> IntLinkedList
        auto *value = reinterpret_cast<IntLinkedList *>(pyValue);

        auto *result = Py_CreateObj<IntLinkedList>(IntLinkedListType, pySelf);
        if (result == nullptr) {
            return PyErr_NoMemory();
        }

        try {
            result->list.insert(result->list.end(), value->list.begin(), value->list.end());
            return reinterpret_cast<PyObject *>(result);
        } catch (const std::exception &e) {
            SAFE_DECREF(result);
            PyErr_SetString(PyExc_RuntimeError, e.what());
            return nullptr;
        }
    }

    // add -> list[int]
    PyObject *selfIntList = IntLinkedList_to_list(pySelf);
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

static PyObject *IntLinkedList_iadd(PyObject *pySelf, PyObject *iterable) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    // fast extend
    if (Py_TYPE(iterable) == &IntLinkedListType) {
        auto *iter = reinterpret_cast<IntLinkedList *>(iterable);
        self->list.insert(self->list.end(), iter->list.begin(), iter->list.end());
        Py_RETURN_NONE;
    }

    // python iterable extend
    PyObject *iter = PyObject_GetIter(iterable);
    if (iter == nullptr) {
        return nullptr;
    }

    // do extend
    PyObject *item;
    while ((item = PyIter_Next(iter)) != nullptr) {
        if (PyErr_Occurred()) {
            SAFE_DECREF(iter);
            SAFE_DECREF(item);
            return nullptr;
        }

        self->list.push_back(PyLong_AsLong(item));
        SAFE_DECREF(item);
    }

    if (PyErr_Occurred()) {
        SAFE_DECREF(iter);
        return nullptr;
    }

    SAFE_DECREF(iter);
    Py_INCREF(pySelf);
    return pySelf;
}


static PyObject *IntLinkedList_mul(PyObject *pySelf, Py_ssize_t n) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    if (n < 0) {
        n = 0;
    }

    auto *result = Py_CreateObj<IntLinkedList>(IntLinkedListType);
    if (result == nullptr) {
        return PyErr_NoMemory();
    }

    if (n == 0) {
        return reinterpret_cast<PyObject *>(result);
    }

    try {
        for (Py_ssize_t i = 0; i < n; ++i) {
            result->list.insert(result->list.end(), self->list.begin(), self->list.end());
        }

        return reinterpret_cast<PyObject *>(result);
    } catch (const std::exception &e) {
        SAFE_DECREF(result);
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static PyObject *IntLinkedList_rmul(PyObject *pySelf, PyObject *pyValue) {
    if (PyLong_Check(pyValue)) {
        Py_ssize_t n = PyLong_AsSsize_t(pyValue);
        if (PyErr_Occurred()) {
            return nullptr;
        }

        return IntLinkedList_mul(pySelf, n);
    }

    PyErr_SetString(PyExc_TypeError, "Expected an integer on the left-hand side of *");
    return nullptr;
}

static PyObject *IntLinkedList_imul(PyObject *pySelf, Py_ssize_t n) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    if (n < 0) {
        n = 0;
    }

    try {
        if (n == 0) {
            self->list.clear();
        } else {
            const auto begin = self->list.begin();
            const auto end = self->list.end();
            for (Py_ssize_t i = 1; i < n; ++i) {
                self->list.insert(self->list.end(), begin, end);
            }
        }

        Py_INCREF(pySelf);
        return pySelf;
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static int IntLinkedList_contains(PyObject *pySelf, PyObject *key) {
    if (!PyLong_Check(key)) {
        return 0;
    }

    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    int value = PyLong_AsLong(key);
    if (PyErr_Occurred()) {
        return -1;
    }

    try {
        if (std::find(self->list.begin(), self->list.end(), value) != self->list.end()) {
            return 1;  // true
        } else {
            return 0;  // false
        }
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }
}

static PyObject *IntLinkedList_reversed(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    auto iter = IntLinkedListIter_create(self, true);
    if (iter == nullptr) return PyErr_NoMemory();
    return reinterpret_cast<PyObject *>(iter);
}

static PyObject *IntLinkedList_reverse(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    Py_BEGIN_ALLOW_THREADS
        std::reverse(self->list.begin(), self->list.end());
    Py_END_ALLOW_THREADS
    Py_RETURN_NONE;
}

static PyObject *IntLinkedList_clear(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    self->list.clear();
    Py_RETURN_NONE;
}

static __forceinline PyObject *IntLinkedList_eq(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    if (!PySequence_Check(pyValue))
        Py_RETURN_FALSE;

    Py_ssize_t seq_size = PySequence_Size(pyValue);
    if (seq_size != static_cast<Py_ssize_t>(self->list.size()))
        Py_RETURN_FALSE;

    auto iter = self->list.begin();
    for (Py_ssize_t i = 0; i < seq_size; i++, iter++) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr)
            return nullptr;

        int self_value = *iter;
        int other_value = PyLong_AsLong(item);
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

static __forceinline PyObject *IntLinkedList_ne(PyObject *pySelf, PyObject *pyValue) {
    PyObject *isEq = IntLinkedList_eq(pySelf, pyValue);
    if (PyObject_IsTrue(isEq)) {
        SAFE_DECREF(isEq);
        Py_RETURN_FALSE;
    } else {
        SAFE_DECREF(isEq);
        Py_RETURN_TRUE;
    }
}

static __forceinline PyObject *IntLinkedList_lt(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    if (!PySequence_Check(pyValue)) {
        Py_RETURN_FALSE;
    }

    Py_ssize_t seqSize = PySequence_Size(pyValue);
    if (seqSize < 0) {
        return nullptr;
    }

    if (self->list.size() != static_cast<size_t>(seqSize)) {
        Py_RETURN_BOOL(self->list.size() < static_cast<size_t>(seqSize))
    }

    auto iter = self->list.begin();
    for (Py_ssize_t i = 0; i < seqSize; i++, iter++) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr) {
            return nullptr;
        }

        int self_value = *iter;
        int other_value = PyLong_AsLong(item);
        SAFE_DECREF(item);
        if (self_value != other_value) Py_RETURN_BOOL(self_value < other_value)
    }

    Py_RETURN_FALSE;
}

static __forceinline PyObject *IntLinkedList_le(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    if (!PySequence_Check(pyValue)) {
        Py_RETURN_FALSE;
    }

    Py_ssize_t seqSize = PySequence_Size(pyValue);
    if (seqSize < 0) {
        return nullptr;
    }

    if (self->list.size() != static_cast<size_t>(seqSize)) {
        Py_RETURN_BOOL(self->list.size() < static_cast<size_t>(seqSize))
    }

    auto iter = self->list.begin();
    for (Py_ssize_t i = 0; i < seqSize; i++, iter++) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr) {
            return nullptr;
        }

        int self_value = *iter;
        int other_value = PyLong_AsLong(item);
        SAFE_DECREF(item);
        if (self_value != other_value) Py_RETURN_BOOL(self_value <= other_value)
    }

    Py_RETURN_FALSE;
}

static __forceinline PyObject *IntLinkedList_gt(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    if (!PySequence_Check(pyValue)) {
        Py_RETURN_FALSE;
    }

    Py_ssize_t seqSize = PySequence_Size(pyValue);
    if (seqSize < 0) {
        return nullptr;
    }

    if (self->list.size() != static_cast<size_t>(seqSize)) {
        Py_RETURN_BOOL(self->list.size() < static_cast<size_t>(seqSize))
    }

    auto iter = self->list.begin();
    for (Py_ssize_t i = 0; i < seqSize; i++, iter++) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr) {
            return nullptr;
        }

        int self_value = *iter;
        int other_value = PyLong_AsLong(item);
        SAFE_DECREF(item);
        if (self_value != other_value) Py_RETURN_BOOL(self_value > other_value)
    }

    Py_RETURN_FALSE;
}

static __forceinline PyObject *IntLinkedList_ge(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    if (!PySequence_Check(pyValue)) {
        Py_RETURN_FALSE;
    }

    Py_ssize_t seqSize = PySequence_Size(pyValue);
    if (seqSize < 0) {
        return nullptr;
    }

    if (self->list.size() != static_cast<size_t>(seqSize)) {
        Py_RETURN_BOOL(self->list.size() < static_cast<size_t>(seqSize))
    }

    auto iter = self->list.begin();
    for (Py_ssize_t i = 0; i < seqSize; i++, iter++) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr) {
            return nullptr;
        }

        int self_value = *iter;
        int other_value = PyLong_AsLong(item);
        SAFE_DECREF(item);
        if (self_value != other_value) Py_RETURN_BOOL(self_value >= other_value)
    }

    Py_RETURN_FALSE;
}

static PyObject *IntLinkedList_compare(PyObject *pySelf, PyObject *pyValue, int op) {
    switch (op) {
        case Py_EQ:  // ==
            return IntLinkedList_eq(pySelf, pyValue);
        case Py_NE:  // !=
            return IntLinkedList_ne(pySelf, pyValue);
        case Py_LT:  // <
            return IntLinkedList_lt(pySelf, pyValue);
        case Py_LE:  // <=
            return IntLinkedList_le(pySelf, pyValue);
        case Py_GT:  // >
            return IntLinkedList_gt(pySelf, pyValue);
        case Py_GE:  // >=
            return IntLinkedList_ge(pySelf, pyValue);
        default:
            PyErr_SetString(PyExc_AssertionError, "Invalid comparison operation.");
            return nullptr;
    }
}

#ifdef IS_PYTHON_39_OR_LATER
static PyObject *IntLinkedList_class_getitem(PyObject *cls, PyObject *item) {
    return Py_GenericAlias(cls, item);
}
#endif

static __forceinline PyObject *IntLinkedList_repr(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntLinkedList *>(pySelf);

    auto &vec = self->list;

    if (vec.empty()) {
        return PyUnicode_FromString("[]");
    }

    size_t size = vec.size();
    auto str = std::string("[");
    str.reserve(size * 4);

    char buffer[32];

    const size_t lastIdx = size - 1;
    auto iter = vec.begin();
    for (size_t i = 0; i < lastIdx; ++i, ++iter) {
        // to string
        int len = snprintf(buffer, sizeof(buffer), "%d", *iter);
        str.append(buffer, len);
        str += ", ";
    }

    int len = snprintf(buffer, sizeof(buffer), "%d", *at(vec, lastIdx));
    str.append(buffer, len);

    str += "]";

    return PyUnicode_FromString(str.c_str());
}

static PyObject *IntLinkedList_str(PyObject *pySelf) {
    return IntLinkedList_repr(pySelf);
}

static PyMethodDef IntLinkedList_methods[] = {
        {"from_range", (PyCFunction) IntLinkedList_from_range, METH_VARARGS | METH_STATIC},
        {"to_list", (PyCFunction) IntLinkedList_to_list, METH_NOARGS},
        {"copy", (PyCFunction) IntLinkedList_copy, METH_NOARGS},
        {"append", (PyCFunction) IntLinkedList_append, METH_O},
        {"extend", (PyCFunction) IntLinkedList_extend, METH_FASTCALL},
        {"pop", (PyCFunction) IntLinkedList_pop, METH_FASTCALL},
        {"index", (PyCFunction) IntLinkedList_index, METH_VARARGS},
        {"count", (PyCFunction) IntLinkedList_count, METH_O},
        {"insert", (PyCFunction) IntLinkedList_insert, METH_VARARGS},
        {"remove", (PyCFunction) IntLinkedList_remove, METH_O},
        {"sort", (PyCFunction) IntLinkedList_sort, METH_VARARGS | METH_KEYWORDS},
        {"reverse", (PyCFunction) IntLinkedList_reverse, METH_NOARGS},
        {"clear", (PyCFunction) IntLinkedList_clear, METH_NOARGS},
        {"__rmul__", (PyCFunction) IntLinkedList_rmul, METH_O},
        {"__reversed__", (PyCFunction) IntLinkedList_reversed, METH_NOARGS},
#ifdef IS_PYTHON_39_OR_LATER
        {"__class_getitem__", (PyCFunction) IntLinkedList_class_getitem, METH_O | METH_CLASS},
#endif
        {nullptr}
};

static struct PyModuleDef IntLinkedList_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.IntLinkedList",
        "An IntLinkedList_module that creates an IntLinkedList",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

static PySequenceMethods IntLinkedList_asSequence = {
        IntLinkedList_len,
        IntLinkedList_add,
        IntLinkedList_mul,
        IntLinkedList_getitem,
        nullptr,
        IntLinkedList_setitem,
        nullptr,
        IntLinkedList_contains,
        IntLinkedList_iadd,
        IntLinkedList_imul
};

static PyMappingMethods IntLinkedList_asMapping = {
        IntLinkedList_len,
        IntLinkedList_getitem_slice,
        IntLinkedList_setitem_slice
};

void initializeIntLinkedListType(PyTypeObject &type) {
    type.tp_name = "IntLinkedList";
    type.tp_basicsize = sizeof(IntLinkedList);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_as_sequence = &IntLinkedList_asSequence;
    type.tp_as_mapping = &IntLinkedList_asMapping;
    type.tp_iter = IntLinkedList_iter;
    type.tp_methods = IntLinkedList_methods;
    type.tp_init = (initproc) IntLinkedList_init;
    type.tp_new = PyType_GenericNew;
    type.tp_dealloc = (destructor) IntLinkedList_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
    type.tp_hash = PyObject_HashNotImplemented;
    type.tp_richcompare = IntLinkedList_compare;
    type.tp_repr = IntLinkedList_repr;
    type.tp_str = IntLinkedList_str;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_IntLinkedList() {
    initializeIntLinkedListType(IntLinkedListType);

    PyObject *object = PyModule_Create(&IntLinkedList_module);
    if (object == nullptr)
        return nullptr;

    Py_INCREF(&IntLinkedListType);
    if (PyModule_AddObject(object, "IntLinkedList", (PyObject *) &IntLinkedListType) < 0) {
        Py_DECREF(&IntLinkedListType);
        Py_DECREF(object);
        return nullptr;
    }

    return object;
}
#pragma clang diagnostic pop

}
