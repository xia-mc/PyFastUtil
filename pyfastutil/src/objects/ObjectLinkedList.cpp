//
// Created by xia__mc on 2024/11/19.
//

#include "ObjectLinkedList.h"
#include <list>
#include <algorithm>
#include <stdexcept>
#include "utils/PythonUtils.h"
#include "utils/include/TimSort.h"
#include "utils/simd/BitonicSort.h"
#include "utils/simd/Utils.h"
#include "utils/memory/AlignedAllocator.h"
#include "objects/ObjectLinkedListIter.h"
#include "utils/include/CPythonSort.h"
#include "utils/Utils.h"

extern "C" {
static PyTypeObject ObjectLinkedListType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
};

static int ObjectLinkedList_init(ObjectLinkedList *self, PyObject *args, PyObject *kwargs) {
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
            if (Py_TYPE(pyIterable) == &ObjectLinkedListType) {  // ObjectLinkedList is a final class
                auto *iter = reinterpret_cast<ObjectLinkedList *>(pyIterable);
                self->list = iter->list;
                return 0;
            }

            if (PyList_Check(pyIterable) || PyTuple_Check(pyIterable)) {  // fast operation
                auto fastKeys = PySequence_Fast(pyIterable, "Shouldn't be happen (ObjectLinkedList).");
                if (fastKeys == nullptr) {
                    return -1;
                }

                const auto size = PySequence_Fast_GET_SIZE(fastKeys);
                auto items = PySequence_Fast_ITEMS(fastKeys);
                for (Py_ssize_t i = 0; i < size; ++i) {
                    PyObject *value = items[i];
                    Py_INCREF(value);
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
                self->list.push_back(item);
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

static void ObjectLinkedList_dealloc(ObjectLinkedList *self) {
    for (PyObject *item: self->list) {
        SAFE_DECREF(item);
    }
    self->list.~list();
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *ObjectLinkedList_to_list(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    const auto size = static_cast<Py_ssize_t>(self->list.size());
    PyObject *result = PyList_New(size);
    if (result == nullptr) return PyErr_NoMemory();

    auto iter = self->list.begin();
    for (Py_ssize_t i = 0; i < size; ++i, ++iter) {
        PyObject *item = *iter;
        if (item == nullptr) {
            SAFE_DECREF(result);
            return nullptr;
        }

        PyList_SET_ITEM(result, i, item);
        Py_INCREF(item);
    }

    return result;
}

static PyObject *ObjectLinkedList_copy(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    auto *copy = Py_CreateObj<ObjectLinkedList>(ObjectLinkedListType);
    if (copy == nullptr) return PyErr_NoMemory();

    try {
        copy->list = self->list;
        for (const auto &item: copy->list) {
            Py_INCREF(item);
        }
    } catch (const std::exception &e) {
        PyObject_Del(copy);
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    return reinterpret_cast<PyObject *>(copy);
}

static PyObject *ObjectLinkedList_append(PyObject *pySelf, PyObject *object) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    PyObject *value = object;
    if (PyErr_Occurred()) {
        return nullptr;
    }

    try {
        self->list.push_back(value);
        Py_INCREF(value);
        self->modCount++;
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    Py_RETURN_NONE;
}

static PyObject *ObjectLinkedList_extend(PyObject *pySelf, PyObject *const *args, const Py_ssize_t nargs) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    // FASTCALL ensure args != nullptr
    if (nargs != 1) {
        PyErr_SetString(PyExc_TypeError, "extend() takes exactly one argument");
        return nullptr;
    }

    PyObject *iterable = args[0];

    // fast extend
    if (Py_TYPE(iterable) == &ObjectLinkedListType) {
        auto *iter = reinterpret_cast<ObjectLinkedList *>(iterable);
        self->list.insert(self->list.end(), iter->list.begin(), iter->list.end());
        for (const auto &item: iter->list) {
            Py_INCREF(item);
        }
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
            return nullptr;
        }

        self->list.push_back(item);
    }
    self->modCount++;

    if (PyErr_Occurred()) {
        SAFE_DECREF(iter);
        return nullptr;
    }

    SAFE_DECREF(iter);
    Py_RETURN_NONE;
}


static PyObject *ObjectLinkedList_pop(PyObject *pySelf, PyObject *const *args, const Py_ssize_t nargs) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

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
        return popped;
    }

    auto popped = at(self->list, index);
    self->list.erase(popped);
    self->modCount++;
    return *popped;
}

static PyObject *ObjectLinkedList_index(PyObject *pySelf, PyObject *args) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    PyObject *value;
    Py_ssize_t start = 0;
    auto stop = static_cast<Py_ssize_t>(self->list.size());

    if (!PyArg_ParseTuple(args, "O|nn", &value, &start, &stop)) {
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

static PyObject *ObjectLinkedList_count(PyObject *pySelf, PyObject *object) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    try {
        size_t result = std::count(self->list.begin(), self->list.end(), object);

        return PyLong_FromSize_t(result);
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static PyObject *ObjectLinkedList_insert(PyObject *pySelf, PyObject *args) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    Py_ssize_t index;
    PyObject *value;

    if (!PyArg_ParseTuple(args, "nO", &index, &value)) {
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
        Py_INCREF(value);
        self->modCount++;
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    Py_RETURN_NONE;
}

static PyObject *ObjectLinkedList_remove(PyObject *pySelf, PyObject *object) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    PyObject *value = object;
    if (PyErr_Occurred()) {
        return nullptr;
    }

    try {
        auto it = std::find(self->list.begin(), self->list.end(), value);

        if (it != self->list.end()) {
            self->list.erase(it);
            SAFE_DECREF(value);
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

static PyObject *ObjectLinkedList_sort(PyObject *pySelf, PyObject *args, PyObject *kwargs) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    PyObject *keyFunc = Py_None;
    int reverseInt = 0;  // default: false
    static constexpr const char *kwlist[] = {"key", "reverse", nullptr};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|Oi", const_cast<char **>(kwlist), &keyFunc, &reverseInt)) {
        return nullptr;
    }

    // costs extra memory
    const auto vecSize = self->list.size();
    auto **pyData = static_cast<PyObject **>(PyMem_Malloc(sizeof(PyObject *) * vecSize));
    if (pyData == nullptr) {
        PyErr_NoMemory();
        return nullptr;
    }

    auto iter = self->list.begin();
    for (size_t i = 0; i < vecSize; ++i, ++iter) {
        pyData[i] = *iter;
    }

    CPython_sort(pyData,
                 static_cast<Py_ssize_t>(vecSize),
                 keyFunc == Py_None ? nullptr : keyFunc, reverseInt);

    iter = self->list.begin();
    for (size_t i = 0; i < vecSize; ++i, ++iter) {
        *iter = pyData[i];
    }

    PyMem_FREE(pyData);
    self->modCount++;

    Py_RETURN_NONE;
}

static Py_ssize_t ObjectLinkedList_len(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    return static_cast<Py_ssize_t>(self->list.size());
}


static PyObject *ObjectLinkedList_iter(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    auto iter = ObjectLinkedListIter_create(self);
    if (iter == nullptr) return PyErr_NoMemory();
    return reinterpret_cast<PyObject *>(iter);
}

static PyObject *ObjectLinkedList_getitem(PyObject *pySelf, Py_ssize_t pyIndex) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    auto size = static_cast<Py_ssize_t>(self->list.size());

    if (pyIndex < 0) {
        pyIndex = size + pyIndex;
    }

    if (pyIndex < 0 || pyIndex >= size) {
        PyErr_SetString(PyExc_IndexError, "index out of range.");
        return nullptr;
    }

    try {
        PyObject *item = *at(self->list, static_cast<size_t>(pyIndex));
        Py_INCREF(item);
        return item;
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static PyObject *ObjectLinkedList_getitem_slice(PyObject *pySelf, PyObject *slice) {
    if (PyIndex_Check(slice)) {
        Py_ssize_t pyIndex = PyNumber_AsSsize_t(slice, PyExc_IndexError);
        if (pyIndex == -1 && PyErr_Occurred()) {
            return nullptr;
        }
        return ObjectLinkedList_getitem(pySelf, pyIndex);
    }

    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

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
        PyObject *item = *at(self->list, static_cast<size_t>(index));
        Py_INCREF(item);
        if (item == nullptr) {
            SAFE_DECREF(result);
            return nullptr;
        }
        PyList_SET_ITEM(result, i, item);
    }
    return result;
}

static int ObjectLinkedList_setitem(PyObject *pySelf, Py_ssize_t pyIndex, PyObject *pyValue) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

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
            SAFE_DECREF(*iter);
            self->list.erase(iter);
        } else {
            PyObject *value = pyValue;
            if (PyErr_Occurred()) {
                return -1;
            }

            *at(self->list, static_cast<size_t>(pyIndex)) = value;
            Py_INCREF(value);
        }
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }
    return 0;
}


static int ObjectLinkedList_setitem_slice(PyObject *pySelf, PyObject *slice, PyObject *value) {
    if (PyIndex_Check(slice)) {
        Py_ssize_t index = PyNumber_AsSsize_t(slice, PyExc_IndexError);
        if (index == -1 && PyErr_Occurred()) {
            return -1;
        }
        return ObjectLinkedList_setitem(pySelf, index, value);
    }

    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

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
                SAFE_DECREF(*iter);
                self->list.erase(iter);
            } else {
                item = PySequence_GetItem(value, i);
                if (item == nullptr) {
                    return -1;
                }

                *at(self->list, static_cast<size_t>(index)) = item;
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

static PyObject *ObjectLinkedList_add(PyObject *pySelf, PyObject *pyValue) {
    if (Py_TYPE(pyValue) == &ObjectLinkedListType) {
        // fast add -> ObjectLinkedList
        auto *value = reinterpret_cast<ObjectLinkedList *>(pyValue);

        auto *result = Py_CreateObj<ObjectLinkedList>(ObjectLinkedListType, pySelf);
        if (result == nullptr) {
            return PyErr_NoMemory();
        }

        try {
            for (const auto &item: value->list) {
                Py_INCREF(item);
            }
            result->list.insert(result->list.end(), value->list.begin(), value->list.end());
            return reinterpret_cast<PyObject *>(result);
        } catch (const std::exception &e) {
            SAFE_DECREF(result);
            PyErr_SetString(PyExc_RuntimeError, e.what());
            return nullptr;
        }
    }

    // add -> list[int]
    PyObject *selfIntList = ObjectLinkedList_to_list(pySelf);
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

static PyObject *ObjectLinkedList_iadd(PyObject *pySelf, PyObject *iterable) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    // fast extend
    if (Py_TYPE(iterable) == &ObjectLinkedListType) {
        auto *iter = reinterpret_cast<ObjectLinkedList *>(iterable);
        for (const auto &item: iter->list) {
            Py_INCREF(item);
        }
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
        PyObject *value = item;

        if (PyErr_Occurred()) {
            SAFE_DECREF(iter);
            return nullptr;
        }

        self->list.push_back(value);
    }

    if (PyErr_Occurred()) {
        SAFE_DECREF(iter);
        return nullptr;
    }

    SAFE_DECREF(iter);
    Py_INCREF(pySelf);
    return pySelf;
}


static PyObject *ObjectLinkedList_mul(PyObject *pySelf, Py_ssize_t n) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    if (n < 0) {
        n = 0;
    }

    auto *result = Py_CreateObj<ObjectLinkedList>(ObjectLinkedListType);
    if (result == nullptr) {
        return PyErr_NoMemory();
    }

    if (n == 0) {
        return reinterpret_cast<PyObject *>(result);
    }

    try {
        for (Py_ssize_t i = 0; i < n; ++i) {
            for (const auto obj: self->list) {
                result->list.push_back(obj);
                Py_INCREF(obj);
            }
        }

        return reinterpret_cast<PyObject *>(result);
    } catch (const std::exception &e) {
        SAFE_DECREF(result);
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static PyObject *ObjectLinkedList_rmul(PyObject *pySelf, PyObject *pyValue) {
    if (PyLong_Check(pyValue)) {
        Py_ssize_t n = PyLong_AsSsize_t(pyValue);
        if (PyErr_Occurred()) {
            return nullptr;
        }

        return ObjectLinkedList_mul(pySelf, n);
    }

    PyErr_SetString(PyExc_TypeError, "Expected an integer on the left-hand side of *");
    return nullptr;
}

static PyObject *ObjectLinkedList_imul(PyObject *pySelf, Py_ssize_t n) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    if (n < 0) {
        n = 0;
    }

    try {
        if (n == 0) {
            self->list.clear();
        } else {
            const auto size = self->list.size();
            for (Py_ssize_t i = 1; i < n; ++i) {
                auto iter = self->list.begin();
                for (size_t j = 0; j < size; ++j, ++iter) {
                    self->list.push_back(*iter);
                    Py_INCREF(*iter);
                }
            }
        }

        Py_INCREF(pySelf);
        return pySelf;
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static int ObjectLinkedList_contains(PyObject *pySelf, PyObject *key) {
    if (!PyLong_Check(key)) {
        return 0;
    }

    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    PyObject *value = key;

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

static PyObject *ObjectLinkedList_reversed(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    auto iter = ObjectLinkedListIter_create(self, true);
    if (iter == nullptr) return PyErr_NoMemory();
    return reinterpret_cast<PyObject *>(iter);
}

static PyObject *ObjectLinkedList_reverse(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    Py_BEGIN_ALLOW_THREADS
        std::reverse(self->list.begin(), self->list.end());
    Py_END_ALLOW_THREADS
    Py_RETURN_NONE;
}

static PyObject *ObjectLinkedList_clear(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    for (auto &item: self->list) {
        SAFE_DECREF(item);
    }
    self->list.clear();
    Py_RETURN_NONE;
}

static __forceinline PyObject *ObjectLinkedList_eq(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

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

        PyObject *self_value = *iter;
        PyObject *other_value = item;

        // compare
        int cmpResult = PyObject_RichCompareBool(self_value, other_value, Py_EQ);
        SAFE_DECREF(item);
        if (cmpResult < 0) {
            return nullptr;
        }

        if (cmpResult == 0) {
            Py_RETURN_FALSE;
        }
    }

    Py_RETURN_TRUE;
}

static __forceinline PyObject *ObjectLinkedList_ne(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    if (!PySequence_Check(pyValue))
        Py_RETURN_TRUE;

    // compare
    Py_ssize_t seq_size = PySequence_Size(pyValue);
    if (seq_size != static_cast<Py_ssize_t>(self->list.size()))
        Py_RETURN_TRUE;

    auto iter = self->list.begin();
    for (Py_ssize_t i = 0; i < seq_size; i++, iter++) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr) return nullptr;
        PyObject *self_value = *iter;
        PyObject *other_value = item;

        int cmpResult = PyObject_RichCompareBool(self_value, other_value, Py_NE);
        SAFE_DECREF(item);
        if (cmpResult < 0) {
            return nullptr;
        }

        if (cmpResult != 0) {
            Py_RETURN_TRUE;
        }
    }

    Py_RETURN_FALSE;
}

static __forceinline PyObject *ObjectLinkedList_lt(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

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

        // compare
        int cmpResult = PyObject_RichCompareBool(*iter, item, Py_LT);
        SAFE_DECREF(item);
        if (cmpResult < 0) {
            return nullptr;
        }
        Py_RETURN_BOOL(cmpResult == 1)  // self->list[i] < item
    }

    Py_RETURN_FALSE;
}

static __forceinline PyObject *ObjectLinkedList_le(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

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

        // compare
        int cmpResult = PyObject_RichCompareBool(*iter, item, Py_LE);
        SAFE_DECREF(item);
        if (cmpResult < 0) {
            return nullptr;
        }
        Py_RETURN_BOOL(cmpResult == 1)  // self->list[i] <= item
    }

    Py_RETURN_FALSE;
}

static __forceinline PyObject *ObjectLinkedList_gt(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

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

        // compare
        int cmpResult = PyObject_RichCompareBool(*iter, item, Py_GT);
        SAFE_DECREF(item);
        if (cmpResult < 0) {
            return nullptr;
        }
        Py_RETURN_BOOL(cmpResult == 1)  // self->list[i] > item
    }

    Py_RETURN_FALSE;
}

static __forceinline PyObject *ObjectLinkedList_ge(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

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

        // compare
        int cmpResult = PyObject_RichCompareBool(*iter, item, Py_GE);
        SAFE_DECREF(item);
        if (cmpResult < 0) {
            return nullptr;
        }
        Py_RETURN_BOOL(cmpResult == 1)  // self->list[i] >= item
    }

    Py_RETURN_FALSE;
}

static PyObject *ObjectLinkedList_compare(PyObject *pySelf, PyObject *pyValue, int op) {
    switch (op) {
        case Py_EQ:  // ==
            return ObjectLinkedList_eq(pySelf, pyValue);
        case Py_NE:  // !=
            return ObjectLinkedList_ne(pySelf, pyValue);
        case Py_LT:  // <
            return ObjectLinkedList_lt(pySelf, pyValue);
        case Py_LE:  // <=
            return ObjectLinkedList_le(pySelf, pyValue);
        case Py_GT:  // >
            return ObjectLinkedList_gt(pySelf, pyValue);
        case Py_GE:  // >=
            return ObjectLinkedList_ge(pySelf, pyValue);
        default:
            PyErr_SetString(PyExc_AssertionError, "Invalid comparison operation.");
            return nullptr;
    }
}

#ifdef IS_PYTHON_39_OR_LATER
static PyObject *ObjectLinkedList_class_getitem(PyObject *cls, PyObject *item) {
    return Py_GenericAlias(cls, item);
}
#endif

static __forceinline PyObject *ObjectLinkedList_repr(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectLinkedList *>(pySelf);

    const auto &vec = self->list;

    if (vec.empty()) {
        return PyUnicode_FromString("[]");
    }

    PyObject *reprList = PyUnicode_FromString("[");
    if (reprList == nullptr) {
        return nullptr;
    }

    // generate string
    auto iter = self->list.begin();
    for (size_t i = 0; i < vec.size() - 1; ++i, ++iter) {
        PyObject *itemRepr = PyObject_Repr(*iter);
        if (itemRepr == nullptr) {
            SAFE_DECREF(reprList);
            return nullptr;
        }

        PyUnicode_AppendAndDel(&reprList, itemRepr);
        PyUnicode_AppendAndDel(&reprList, PyUnicode_FromString(", "));
    }

    PyObject *itemRepr = PyObject_Repr(vec.back());
    PyUnicode_AppendAndDel(&reprList, itemRepr);
    PyUnicode_AppendAndDel(&reprList, PyUnicode_FromString("]"));

    return reprList;
}

static PyObject *ObjectLinkedList_str(PyObject *pySelf) {
    return ObjectLinkedList_repr(pySelf);
}

static PyMethodDef ObjectLinkedList_methods[] = {
        {"to_list", (PyCFunction) ObjectLinkedList_to_list, METH_NOARGS},
        {"copy", (PyCFunction) ObjectLinkedList_copy, METH_NOARGS},
        {"append", (PyCFunction) ObjectLinkedList_append, METH_O},
        {"extend", (PyCFunction) ObjectLinkedList_extend, METH_FASTCALL},
        {"pop", (PyCFunction) ObjectLinkedList_pop, METH_FASTCALL},
        {"index", (PyCFunction) ObjectLinkedList_index, METH_VARARGS},
        {"count", (PyCFunction) ObjectLinkedList_count, METH_O},
        {"insert", (PyCFunction) ObjectLinkedList_insert, METH_VARARGS},
        {"remove", (PyCFunction) ObjectLinkedList_remove, METH_O},
        {"sort", (PyCFunction) ObjectLinkedList_sort, METH_VARARGS | METH_KEYWORDS},
        {"reverse", (PyCFunction) ObjectLinkedList_reverse, METH_NOARGS},
        {"clear", (PyCFunction) ObjectLinkedList_clear, METH_NOARGS},
        {"__rmul__", (PyCFunction) ObjectLinkedList_rmul, METH_O},
        {"__reversed__", (PyCFunction) ObjectLinkedList_reversed, METH_NOARGS},
#ifdef IS_PYTHON_39_OR_LATER
        {"__class_getitem__", (PyCFunction) ObjectLinkedList_class_getitem, METH_O | METH_CLASS},
#endif
        {nullptr}
};

static struct PyModuleDef ObjectLinkedList_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.ObjectLinkedList",
        "An ObjectLinkedList_module that creates an ObjectLinkedList",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

static PySequenceMethods ObjectLinkedList_asSequence = {
        ObjectLinkedList_len,
        ObjectLinkedList_add,
        ObjectLinkedList_mul,
        ObjectLinkedList_getitem,
        nullptr,
        ObjectLinkedList_setitem,
        nullptr,
        ObjectLinkedList_contains,
        ObjectLinkedList_iadd,
        ObjectLinkedList_imul
};

static PyMappingMethods ObjectLinkedList_asMapping = {
        ObjectLinkedList_len,
        ObjectLinkedList_getitem_slice,
        ObjectLinkedList_setitem_slice
};

void initializeObjectLinkedListType(PyTypeObject &type) {
    type.tp_name = "ObjectLinkedList";
    type.tp_basicsize = sizeof(ObjectLinkedList);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_as_sequence = &ObjectLinkedList_asSequence;
    type.tp_as_mapping = &ObjectLinkedList_asMapping;
    type.tp_iter = ObjectLinkedList_iter;
    type.tp_methods = ObjectLinkedList_methods;
    type.tp_init = (initproc) ObjectLinkedList_init;
    type.tp_new = PyType_GenericNew;
    type.tp_dealloc = (destructor) ObjectLinkedList_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
    type.tp_hash = PyObject_HashNotImplemented;
    type.tp_richcompare = ObjectLinkedList_compare;
    type.tp_repr = ObjectLinkedList_repr;
    type.tp_str = ObjectLinkedList_str;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_ObjectLinkedList() {
    initializeObjectLinkedListType(ObjectLinkedListType);

    PyObject *object = PyModule_Create(&ObjectLinkedList_module);
    if (object == nullptr)
        return nullptr;

    Py_INCREF(&ObjectLinkedListType);
    if (PyModule_AddObject(object, "ObjectLinkedList", (PyObject *) &ObjectLinkedListType) < 0) {
        Py_DECREF(&ObjectLinkedListType);
        Py_DECREF(object);
        return nullptr;
    }

    return object;
}
#pragma clang diagnostic pop

}
