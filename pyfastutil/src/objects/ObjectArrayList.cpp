//
// Created by xia__mc on 2024/11/17.
//

#include "ObjectArrayList.h"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "utils/PythonUtils.h"
#include "utils/include/TimSort.h"
#include "utils/simd/BitonicSort.h"
#include "utils/simd/SIMDUtils.h"
#include "utils/memory/AlignedAllocator.h"
#include "utils/memory/FastMemcpy.h"
#include "objects/ObjectArrayListIter.h"
#include "utils/include/CPythonSort.h"

extern "C" {
static PyTypeObject ObjectArrayListType = {
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

static int ObjectArrayList_init(ObjectArrayList *self, PyObject *args, PyObject *kwargs) {
    new(&self->vector) std::vector<PyObject *>();

    PyObject *pyIterable = nullptr;
    Py_ssize_t pySize = -1;

    parseArgs(args, kwargs, pyIterable, pySize);

    // init vector
    try {
        if (pySize > 0) {
            self->vector.reserve(static_cast<size_t>(pySize));
        }

        if (pyIterable != nullptr) {
            if (Py_TYPE(pyIterable) == &ObjectArrayListType) {  // ObjectArrayList is a final class
                auto *iter = reinterpret_cast<ObjectArrayList *>(pyIterable);
                self->vector = iter->vector;
                return 0;
            }

            if (PyList_Check(pyIterable) || PyTuple_Check(pyIterable)) {  // fast operation
                auto fastIter = PySequence_Fast(pyIterable, "Shouldn't be happen (ObjectArrayList).");
                if (fastIter == nullptr) {
                    return -1;
                }

                const auto size = PySequence_Fast_GET_SIZE(fastIter);
                auto items = PySequence_Fast_ITEMS(fastIter);

                for (Py_ssize_t i = 0; i < size; ++i) {
                    Py_INCREF(items[i]);
                }

                self->vector.insert(self->vector.end(), items, items + size);  // even faster than memcpy!!

                SAFE_DECREF(fastIter);
                return 0;
            }

            PyObject *iter = PyObject_GetIter(pyIterable);
            if (iter == nullptr) {
                PyErr_SetString(PyExc_TypeError, "Arg '__iterable' is not iterable.");
                return -1;
            }

            PyObject *item;
            while ((item = PyIter_Next(iter)) != nullptr) {
                self->vector.push_back(item);
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

static void ObjectArrayList_dealloc(ObjectArrayList *self) {
    for (PyObject *item: self->vector) {
        SAFE_DECREF(item);
    }
    self->vector.~vector();
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *ObjectArrayList_resize(PyObject *pySelf, PyObject *pySize) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

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
        const auto newSize = static_cast<size_t>(pySSize);
        self->vector.resize(newSize, Py_None);
        // No need to call Py_INCREF on Py_None or other constant objects
        // because, the reference count operations on global singleton objects
        // like None, True, and False have been optimized to be no-ops.
//        if (newSize > size) {
//            const auto addedNone = newSize - size;
//            for (size_t i = 0; i < addedNone; ++i) {
//                Py_INCREF(Py_None);
//            }
//        }
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    Py_RETURN_NONE;
}

static PyObject *ObjectArrayList_to_list(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    const auto size = static_cast<Py_ssize_t>(self->vector.size());
    auto *result = reinterpret_cast<PyListObject *>(PyList_New(size));
    if (result == nullptr) return PyErr_NoMemory();

    for (const auto &item: self->vector) {
        Py_INCREF(item);
    }

    result->allocated = size;
    fast_memcpy(result->ob_item, self->vector.data(), size * sizeof(PyObject *));

    return reinterpret_cast<PyObject*>(result);
}

static PyObject *ObjectArrayList_copy(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    auto *copy = Py_CreateObj<ObjectArrayList>(ObjectArrayListType);
    if (copy == nullptr) return PyErr_NoMemory();

    try {
        copy->vector = self->vector;
        for (const auto &item: copy->vector) {
            Py_INCREF(item);
        }
    } catch (const std::exception &e) {
        PyObject_Del(copy);
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    return reinterpret_cast<PyObject *>(copy);
}

static PyObject *ObjectArrayList_append(PyObject *pySelf, PyObject *object) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    PyObject *value = object;
    if (PyErr_Occurred()) {
        return nullptr;
    }

    try {
        self->vector.push_back(value);
        Py_INCREF(value);
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    Py_RETURN_NONE;
}

static PyObject *ObjectArrayList_extend(PyObject *pySelf, PyObject *const *args, const Py_ssize_t nargs) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    // FASTCALL ensure args != nullptr
    if (nargs != 1) {
        PyErr_SetString(PyExc_TypeError, "extend() takes exactly one argument");
        return nullptr;
    }

    PyObject *iterable = args[0];

    // fast extend
    if (Py_TYPE(iterable) == &ObjectArrayListType) {
        auto *iter = reinterpret_cast<ObjectArrayList *>(iterable);
        self->vector.insert(self->vector.end(), iter->vector.begin(), iter->vector.end());
        for (const auto &item: iter->vector) {
            Py_INCREF(item);
        }
        Py_RETURN_NONE;
    }

    if (Py_TYPE(iterable) == &PyList_Type) {
        const auto *iter = reinterpret_cast<PyListObject *>(iterable);
        const auto last = iter->ob_item + PyList_GET_SIZE(iter);
        self->vector.insert(self->vector.end(), iter->ob_item, last);
        for (PyObject **item = iter->ob_item; item < last; ++item) {
            Py_INCREF(*item);
        }
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
        if (PyErr_Occurred()) {
            SAFE_DECREF(iter);
            return nullptr;
        }

        self->vector.push_back(item);
    }

    if (PyErr_Occurred()) {
        SAFE_DECREF(iter);
        return nullptr;
    }

    SAFE_DECREF(iter);
    Py_RETURN_NONE;
}


static PyObject *ObjectArrayList_pop(PyObject *pySelf, PyObject *const *args, const Py_ssize_t nargs) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    if (self->vector.empty()) {
        PyErr_SetString(PyExc_IndexError, "pop from empty list");
        return nullptr;
    }

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

        return popped;
    }

    const auto popped = self->vector[static_cast<size_t>(index)];
    self->vector.erase(self->vector.begin() + index);

    return popped;
}

static PyObject *ObjectArrayList_index(PyObject *pySelf, PyObject *args) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    PyObject *value;
    Py_ssize_t start = 0;
    auto stop = static_cast<Py_ssize_t>(self->vector.size());

    if (!PyArg_ParseTuple(args, "O|nn", &value, &start, &stop)) {
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

static PyObject *ObjectArrayList_count(PyObject *pySelf, PyObject *object) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    try {
        size_t result = std::count(self->vector.begin(), self->vector.end(), object);

        return PyLong_FromSize_t(result);
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static PyObject *ObjectArrayList_insert(PyObject *pySelf, PyObject *args) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    Py_ssize_t index;
    PyObject *value;

    if (!PyArg_ParseTuple(args, "nO", &index, &value)) {
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
        Py_INCREF(value);
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    Py_RETURN_NONE;
}

static PyObject *ObjectArrayList_remove(PyObject *pySelf, PyObject *object) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    PyObject *value = object;
    if (PyErr_Occurred()) {
        return nullptr;
    }

    try {
        const auto end = self->vector.end();
        const auto it = std::find(self->vector.begin(), end, value);

        if (it != end) {
            self->vector.erase(it);
            SAFE_DECREF(value);
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

static PyObject *ObjectArrayList_sort(PyObject *pySelf, PyObject *args, PyObject *kwargs) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    PyObject *keyFunc = Py_None;
    int reverseInt = 0;  // default: false
    static constexpr const char *kwlist[] = {"key", "reverse", nullptr};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|Oi", const_cast<char **>(kwlist), &keyFunc, &reverseInt)) {
        return nullptr;
    }

    // do sort
    return CPython_sort(self->vector.data(),
                        static_cast<Py_ssize_t>(self->vector.size()),
                        keyFunc == Py_None ? nullptr : keyFunc, reverseInt);
}

static Py_ssize_t ObjectArrayList_len(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    return static_cast<Py_ssize_t>(self->vector.size());
}


static PyObject *ObjectArrayList_iter(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    auto iter = ObjectArrayListIter_create(self);
    if (iter == nullptr) return PyErr_NoMemory();
    return reinterpret_cast<PyObject *>(iter);
}

static PyObject *ObjectArrayList_getitem(PyObject *pySelf, Py_ssize_t pyIndex) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    auto size = static_cast<Py_ssize_t>(self->vector.size());

    if (pyIndex < 0) {
        pyIndex = size + pyIndex;
    }

    if (pyIndex < 0 || pyIndex >= size) {
        PyErr_SetString(PyExc_IndexError, "index out of range.");
        return nullptr;
    }

    PyObject *item = *(self->vector.data() + pyIndex);
    Py_INCREF(item);
    return item;
}

static PyObject *ObjectArrayList_getitem_slice(PyObject *pySelf, PyObject *slice) {
    if (PyIndex_Check(slice)) {
        Py_ssize_t pyIndex = PyNumber_AsSsize_t(slice, PyExc_IndexError);
        if (pyIndex == -1 && PyErr_Occurred()) {
            return nullptr;
        }
        return ObjectArrayList_getitem(pySelf, pyIndex);
    }

    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

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
        PyObject *item = *(self->vector.data() + index);
        Py_INCREF(item);
        PyList_SET_ITEM(result, i, item);
    }
    return result;
}

static int ObjectArrayList_setitem(PyObject *pySelf, Py_ssize_t pyIndex, PyObject *pyValue) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

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
            SAFE_DECREF(self->vector[static_cast<size_t>(pyIndex)]);
            self->vector.erase(self->vector.begin() + pyIndex);
        } else {
            PyObject *value = pyValue;
            if (PyErr_Occurred()) {
                return -1;
            }

            self->vector[static_cast<size_t>(pyIndex)] = value;
            Py_INCREF(value);
        }
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }
    return 0;
}

static int ObjectArrayList_setitem_slice(PyObject *pySelf, PyObject *slice, PyObject *value) {
    if (PyIndex_Check(slice)) {
        Py_ssize_t index = PyNumber_AsSsize_t(slice, PyExc_IndexError);
        if (index == -1 && PyErr_Occurred()) {
            return -1;
        }
        return ObjectArrayList_setitem(pySelf, index, value);
    }

    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

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

    try {
        PyObject **startPtr = self->vector.data() + start;

        // Helper function to handle fast memcpy and reference counting
        static const auto fastCopy = [](PyObject **dest, PyObject **src, Py_ssize_t length) {
            for (Py_ssize_t i = 0; i < length; i++) {
                SAFE_DECREF(dest[i]);  // Decrease reference for old item
                Py_INCREF(src[i]);    // Increase reference for new item
            }
            fast_memcpy(dest, src, length * sizeof(PyObject *));
        };

        // Helper function to handle generic copy (non-optimized path)
        static const auto genericCopy = [](PyObject **dest, PyObject *srcSeq, Py_ssize_t length) {
            for (Py_ssize_t i = 0; i < length; i++) {
                PyObject *item = PySequence_GetItem(srcSeq, i);
                if (item == nullptr) {
                    return -1;  // Exception already set
                }
                SAFE_DECREF(dest[i]);
                dest[i] = item;
            }
            return 0;
        };

        if (newLength == sliceLength) {  // Case 1: Replace elements
            if (Py_TYPE(value) == &ObjectArrayListType) {
                auto *fastValue = reinterpret_cast<ObjectArrayList *>(value);
                fastCopy(startPtr, fastValue->vector.data(), sliceLength);
            } else if (Py_TYPE(value) == &PyList_Type) {
                auto *fastValue = reinterpret_cast<PyListObject *>(value);
                fastCopy(startPtr, fastValue->ob_item, sliceLength);
            } else {
                if (genericCopy(startPtr, value, sliceLength) < 0) {
                    return -1;
                }
            }
        } else if (newLength < sliceLength) {  // Case 2: Replace and remove extra elements
            if (Py_TYPE(value) == &ObjectArrayListType) {
                auto *fastValue = reinterpret_cast<ObjectArrayList *>(value);
                fastCopy(startPtr, fastValue->vector.data(), newLength);
            } else if (Py_TYPE(value) == &PyList_Type) {
                auto *fastValue = reinterpret_cast<PyListObject *>(value);
                fastCopy(startPtr, fastValue->ob_item, newLength);
            } else {
                if (genericCopy(startPtr, value, newLength) < 0) {
                    return -1;
                }
            }

            // Remove extra elements
            for (Py_ssize_t i = newLength; i < sliceLength; i++) {
                SAFE_DECREF(startPtr[i]);
            }
            self->vector.erase(self->vector.begin() + start + newLength, self->vector.begin() + stop);
        } else {  // Case 3: Replace and insert additional elements
            if (Py_TYPE(value) == &ObjectArrayListType) {
                auto *fastValue = reinterpret_cast<ObjectArrayList *>(value);
                fastCopy(startPtr, fastValue->vector.data(), sliceLength);
                self->vector.insert(
                        self->vector.begin() + start + sliceLength,
                        fastValue->vector.data() + sliceLength,
                        fastValue->vector.data() + newLength
                );
            } else if (Py_TYPE(value) == &PyList_Type) {
                auto *fastValue = reinterpret_cast<PyListObject *>(value);
                fastCopy(startPtr, fastValue->ob_item, sliceLength);
                self->vector.insert(
                        self->vector.begin() + start + sliceLength,
                        fastValue->ob_item + sliceLength,
                        fastValue->ob_item + newLength
                );
            } else {
                if (genericCopy(startPtr, value, sliceLength) < 0) {
                    return -1;
                }

                // Insert additional elements
                for (Py_ssize_t i = sliceLength; i < newLength; i++) {
                    PyObject *item = PySequence_GetItem(value, i);
                    if (item == nullptr) {
                        return -1;
                    }
                    self->vector.insert(self->vector.begin() + start + i, item);
                }
            }
        }
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }

    return 0;
}

static PyObject *ObjectArrayList_add(PyObject *pySelf, PyObject *pyValue) {
    if (Py_TYPE(pyValue) == &ObjectArrayListType) {
        // fast add -> ObjectArrayList
        auto *value = reinterpret_cast<ObjectArrayList *>(pyValue);

        auto *result = Py_CreateObj<ObjectArrayList>(ObjectArrayListType, pySelf);
        if (result == nullptr) {
            return PyErr_NoMemory();
        }

        try {
            for (const auto &item: value->vector) {
                Py_INCREF(item);
            }
            result->vector.insert(result->vector.end(), value->vector.begin(), value->vector.end());
            return reinterpret_cast<PyObject *>(result);
        } catch (const std::exception &e) {
            SAFE_DECREF(result);
            PyErr_SetString(PyExc_RuntimeError, e.what());
            return nullptr;
        }
    }

    // add -> list[int]
    PyObject *selfIntList = ObjectArrayList_to_list(pySelf);
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

static PyObject *ObjectArrayList_iadd(PyObject *pySelf, PyObject *iterable) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    // fast extend
    if (Py_TYPE(iterable) == &ObjectArrayListType) {
        auto *iter = reinterpret_cast<ObjectArrayList *>(iterable);
        for (const auto &item: iter->vector) {
            Py_INCREF(item);
        }
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
        PyObject *value = item;

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


static PyObject *ObjectArrayList_mul(PyObject *pySelf, Py_ssize_t n) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    if (n < 0) {
        n = 0;
    }

    auto *result = Py_CreateObj<ObjectArrayList>(ObjectArrayListType);
    if (result == nullptr) {
        return PyErr_NoMemory();
    }

    if (n == 0) {
        return reinterpret_cast<PyObject *>(result);
    }

    try {
        const auto selfSize = self->vector.size();

        result->vector.reserve(selfSize * n);
        for (Py_ssize_t i = 0; i < n; ++i) {
            for (const auto obj: self->vector) {
                result->vector.push_back(obj);
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

static PyObject *ObjectArrayList_rmul(PyObject *pySelf, PyObject *pyValue) {
    if (PyLong_Check(pyValue)) {
        Py_ssize_t n = PyLong_AsSsize_t(pyValue);
        if (PyErr_Occurred()) {
            return nullptr;
        }

        return ObjectArrayList_mul(pySelf, n);
    }

    PyErr_SetString(PyExc_TypeError, "Expected an integer on the left-hand side of *");
    return nullptr;
}

static PyObject *ObjectArrayList_imul(PyObject *pySelf, Py_ssize_t n) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    if (n < 0) {
        n = 0;
    }

    try {
        if (n == 0) {
            self->vector.clear();
        } else {
            const auto selfSize = self->vector.size();

            self->vector.reserve(selfSize * n);
            for (Py_ssize_t i = 1; i < n; ++i) {
                for (size_t j = 0; j < self->vector.size(); ++j) {
                    self->vector.push_back(self->vector[j]);
                    Py_INCREF(self->vector[j]);
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

static int ObjectArrayList_contains(PyObject *pySelf, PyObject *key) {
    if (!PyLong_Check(key)) {
        return 0;
    }

    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    PyObject *value = key;
    if (PyErr_Occurred()) {
        return -1;
    }

    try {
        if (std::find(self->vector.begin(), self->vector.end(), value) != self->vector.end()) {
            return 1;  // true
        } else {
            return 0;  // false
        }
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }
}

static PyObject *ObjectArrayList_reversed(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    auto iter = ObjectArrayListIter_create(self, true);
    if (iter == nullptr) return PyErr_NoMemory();
    return reinterpret_cast<PyObject *>(iter);
}

static PyObject *ObjectArrayList_reverse(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    Py_BEGIN_ALLOW_THREADS
        simd::simdReverse(self->vector.data(), self->vector.size());
    Py_END_ALLOW_THREADS
    Py_RETURN_NONE;
}

static PyObject *ObjectArrayList_clear(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    for (auto &item: self->vector) {
        SAFE_DECREF(item);
    }
    self->vector.clear();
    Py_RETURN_NONE;
}

static __forceinline PyObject *ObjectArrayList_eq(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    if (!PySequence_Check(pyValue))
        Py_RETURN_FALSE;

    Py_ssize_t seq_size = PySequence_Size(pyValue);
    if (seq_size != static_cast<Py_ssize_t>(self->vector.size()))
        Py_RETURN_FALSE;

    for (Py_ssize_t i = 0; i < seq_size; i++) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr)
            return nullptr;

        PyObject *self_value = self->vector[i];
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

static __forceinline PyObject *ObjectArrayList_ne(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    if (!PySequence_Check(pyValue))
        Py_RETURN_TRUE;

    // compare
    Py_ssize_t seq_size = PySequence_Size(pyValue);
    if (seq_size != static_cast<Py_ssize_t>(self->vector.size()))
        Py_RETURN_TRUE;

    for (Py_ssize_t i = 0; i < seq_size; i++) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr) return nullptr;
        PyObject *self_value = self->vector[i];
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

static __forceinline PyObject *ObjectArrayList_lt(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    if (!PySequence_Check(pyValue)) {
        Py_RETURN_FALSE;
    }

    Py_ssize_t seqSize = PySequence_Size(pyValue);
    if (seqSize < 0) {
        return nullptr;
    }

    if (self->vector.size() != static_cast<size_t>(seqSize)) {
        Py_RETURN_BOOL(self->vector.size() < static_cast<size_t>(seqSize))
    }

    for (Py_ssize_t i = 0; i < seqSize; ++i) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr) {
            return nullptr;
        }

        // compare
        int cmpResult = PyObject_RichCompareBool(self->vector[i], item, Py_LT);
        SAFE_DECREF(item);
        if (cmpResult < 0) {
            return nullptr;
        }
        Py_RETURN_BOOL(cmpResult == 1)  // self->vector[i] < item
    }

    Py_RETURN_FALSE;
}

static __forceinline PyObject *ObjectArrayList_le(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    if (!PySequence_Check(pyValue)) {
        Py_RETURN_FALSE;
    }

    Py_ssize_t seqSize = PySequence_Size(pyValue);
    if (seqSize < 0) {
        return nullptr;
    }

    if (self->vector.size() != static_cast<size_t>(seqSize)) {
        Py_RETURN_BOOL(self->vector.size() < static_cast<size_t>(seqSize))
    }

    for (Py_ssize_t i = 0; i < seqSize; ++i) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr) {
            return nullptr;
        }

        // compare
        int cmpResult = PyObject_RichCompareBool(self->vector[i], item, Py_LE);
        SAFE_DECREF(item);
        if (cmpResult < 0) {
            return nullptr;
        }
        Py_RETURN_BOOL(cmpResult == 1)  // self->vector[i] <= item
    }

    Py_RETURN_FALSE;
}

static __forceinline PyObject *ObjectArrayList_gt(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    if (!PySequence_Check(pyValue)) {
        Py_RETURN_FALSE;
    }

    Py_ssize_t seqSize = PySequence_Size(pyValue);
    if (seqSize < 0) {
        return nullptr;
    }

    if (self->vector.size() != static_cast<size_t>(seqSize)) {
        Py_RETURN_BOOL(self->vector.size() < static_cast<size_t>(seqSize))
    }

    for (Py_ssize_t i = 0; i < seqSize; ++i) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr) {
            return nullptr;
        }

        // compare
        int cmpResult = PyObject_RichCompareBool(self->vector[i], item, Py_GT);
        SAFE_DECREF(item);
        if (cmpResult < 0) {
            return nullptr;
        }
        Py_RETURN_BOOL(cmpResult == 1)  // self->vector[i] > item
    }

    Py_RETURN_FALSE;
}

static __forceinline PyObject *ObjectArrayList_ge(PyObject *pySelf, PyObject *pyValue) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    if (!PySequence_Check(pyValue)) {
        Py_RETURN_FALSE;
    }

    Py_ssize_t seqSize = PySequence_Size(pyValue);
    if (seqSize < 0) {
        return nullptr;
    }

    if (self->vector.size() != static_cast<size_t>(seqSize)) {
        Py_RETURN_BOOL(self->vector.size() < static_cast<size_t>(seqSize))
    }

    for (Py_ssize_t i = 0; i < seqSize; ++i) {
        PyObject *item = PySequence_GetItem(pyValue, i);
        if (item == nullptr) {
            return nullptr;
        }

        // compare
        int cmpResult = PyObject_RichCompareBool(self->vector[i], item, Py_GE);
        SAFE_DECREF(item);
        if (cmpResult < 0) {
            return nullptr;
        }
        Py_RETURN_BOOL(cmpResult == 1)  // self->vector[i] >= item
    }

    Py_RETURN_FALSE;
}

static PyObject *ObjectArrayList_compare(PyObject *pySelf, PyObject *pyValue, int op) {
    switch (op) {
        case Py_EQ:  // ==
            return ObjectArrayList_eq(pySelf, pyValue);
        case Py_NE:  // !=
            return ObjectArrayList_ne(pySelf, pyValue);
        case Py_LT:  // <
            return ObjectArrayList_lt(pySelf, pyValue);
        case Py_LE:  // <=
            return ObjectArrayList_le(pySelf, pyValue);
        case Py_GT:  // >
            return ObjectArrayList_gt(pySelf, pyValue);
        case Py_GE:  // >=
            return ObjectArrayList_ge(pySelf, pyValue);
        default:
            PyErr_SetString(PyExc_AssertionError, "Invalid comparison operation.");
            return nullptr;
    }
}

#ifdef IS_PYTHON_39_OR_LATER
static PyObject *ObjectArrayList_class_getitem(PyObject *cls, PyObject *item) {
    return Py_GenericAlias(cls, item);
}
#endif

static __forceinline PyObject *ObjectArrayList_repr(PyObject *pySelf) {
    auto *self = reinterpret_cast<ObjectArrayList *>(pySelf);

    const auto &vec = self->vector;

    if (vec.empty()) {
        return PyUnicode_FromString("[]");
    }

    PyObject *reprList = PyUnicode_FromString("[");
    if (reprList == nullptr) {
        return nullptr;
    }

    // generate string
    for (size_t i = 0; i < vec.size() - 1; ++i) {
        PyObject *itemRepr = PyObject_Repr(vec[i]);
        if (itemRepr == nullptr) {
            SAFE_DECREF(reprList);
            return nullptr;
        }

        PyUnicode_AppendAndDel(&reprList, itemRepr);
        PyUnicode_AppendAndDel(&reprList, PyUnicode_FromString(", "));
    }

    PyObject *itemRepr = PyObject_Repr(vec[vec.size() - 1]);
    PyUnicode_AppendAndDel(&reprList, itemRepr);
    PyUnicode_AppendAndDel(&reprList, PyUnicode_FromString("]"));

    return reprList;
}

static PyObject *ObjectArrayList_str(PyObject *pySelf) {
    return ObjectArrayList_repr(pySelf);
}

static PyMethodDef ObjectArrayList_methods[] = {
        {"resize", (PyCFunction) ObjectArrayList_resize, METH_O},
        {"to_list", (PyCFunction) ObjectArrayList_to_list, METH_NOARGS},
        {"copy", (PyCFunction) ObjectArrayList_copy, METH_NOARGS},
        {"append", (PyCFunction) ObjectArrayList_append, METH_O},
        {"extend", (PyCFunction) ObjectArrayList_extend, METH_FASTCALL},
        {"pop", (PyCFunction) ObjectArrayList_pop, METH_FASTCALL},
        {"index", (PyCFunction) ObjectArrayList_index, METH_VARARGS},
        {"count", (PyCFunction) ObjectArrayList_count, METH_O},
        {"insert", (PyCFunction) ObjectArrayList_insert, METH_VARARGS},
        {"remove", (PyCFunction) ObjectArrayList_remove, METH_O},
        {"sort", (PyCFunction) ObjectArrayList_sort, METH_VARARGS | METH_KEYWORDS},
        {"reverse", (PyCFunction) ObjectArrayList_reverse, METH_NOARGS},
        {"clear", (PyCFunction) ObjectArrayList_clear, METH_NOARGS},
        {"__rmul__", (PyCFunction) ObjectArrayList_rmul, METH_O},
        {"__reversed__", (PyCFunction) ObjectArrayList_reversed, METH_NOARGS},
#ifdef IS_PYTHON_39_OR_LATER
        {"__class_getitem__", (PyCFunction) ObjectArrayList_class_getitem, METH_O | METH_CLASS},
#endif
        {nullptr}
};

static struct PyModuleDef ObjectArrayList_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.ObjectArrayList",
        "An ObjectArrayList_module that creates an ObjectArrayList",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

static PySequenceMethods ObjectArrayList_asSequence = {
        ObjectArrayList_len,
        ObjectArrayList_add,
        ObjectArrayList_mul,
        ObjectArrayList_getitem,
        nullptr,
        ObjectArrayList_setitem,
        nullptr,
        ObjectArrayList_contains,
        ObjectArrayList_iadd,
        ObjectArrayList_imul
};

static PyMappingMethods ObjectArrayList_asMapping = {
        ObjectArrayList_len,
        ObjectArrayList_getitem_slice,
        ObjectArrayList_setitem_slice
};

void initializeObjectArrayListType(PyTypeObject &type) {
    type.tp_name = "ObjectArrayList";
    type.tp_basicsize = sizeof(ObjectArrayList);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_as_sequence = &ObjectArrayList_asSequence;
    type.tp_as_mapping = &ObjectArrayList_asMapping;
    type.tp_iter = ObjectArrayList_iter;
    type.tp_methods = ObjectArrayList_methods;
    type.tp_init = (initproc) ObjectArrayList_init;
    type.tp_new = PyType_GenericNew;
    type.tp_dealloc = (destructor) ObjectArrayList_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
    type.tp_hash = PyObject_HashNotImplemented;
    type.tp_richcompare = ObjectArrayList_compare;
    type.tp_repr = ObjectArrayList_repr;
    type.tp_str = ObjectArrayList_str;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_ObjectArrayList() {
    initializeObjectArrayListType(ObjectArrayListType);

    PyObject *object = PyModule_Create(&ObjectArrayList_module);
    if (object == nullptr)
        return nullptr;

    Py_INCREF(&ObjectArrayListType);
    if (PyModule_AddObject(object, "ObjectArrayList", (PyObject *) &ObjectArrayListType) < 0) {
        Py_DECREF(&ObjectArrayListType);
        Py_DECREF(object);
        return nullptr;
    }

    return object;
}
#pragma clang diagnostic pop

}
