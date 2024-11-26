//
// Created by xia__mc on 2024/11/24.
//

#include "IntIntHashMap.h"
#include "utils/PythonUtils.h"


extern "C" {

static PyTypeObject IntIntHashMapType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
};

static __forceinline int convert(PyObject *&obj) {  // maybe I will take it to IntArrayList or something else
    return PyLong_Check(obj) ? PyFast_AsInt(obj) : PyLong_AsLong(obj);
}

static int IntIntHashMap_init(IntIntHashMap *self, PyObject *args, PyObject *kwargs) {
    new(&self->map) ankerl::unordered_dense::map<int, int>();

    IntIntHashMap *map = nullptr;
    PyObject *pyDict = nullptr;
    PyObject *pyMapping = nullptr;

    PyObject *pyIterable = nullptr;
    PyObject *extraKwargs = nullptr;

    // parse args
    if (PyTuple_GET_SIZE(args) > 1) {
        PyErr_SetString(PyExc_TypeError, "expected at most 1 argument");
        return -1;
    }

    static constexpr const char *kwlist[] = {"__map", "__iterable", nullptr};

    PyObject *arg1 = nullptr;
    PyObject *arg2 = nullptr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OO", const_cast<char **>(kwlist), &arg1, &arg2, &extraKwargs)) {
        return -1;
    }

    static const auto keyAttr = PyUnicode_FromString("keys");

    if (arg1 != nullptr) {
        if (Py_TYPE(arg1) == &IntIntHashMapType) {
            map = reinterpret_cast<IntIntHashMap *>(arg1);
            pyIterable = arg2;
        } else if (PyDict_Check(arg1)) {
            pyDict = arg1;
            pyIterable = arg2;
        } else if (PyIter_Check(arg1)) {
            pyIterable = arg1;
            pyMapping = arg2;
        } else if (PyMapping_Check(arg1) && PyObject_HasAttr(arg1, keyAttr)) {
            pyMapping = arg1;
            pyIterable = arg2;
        } else {
            PyErr_SetString(PyExc_TypeError, "expected a iterable or a mapping");
            return -1;
        }
    }

    // init map
    static const auto processExtraKwargs = [&self, &extraKwargs]() -> int {
        if (extraKwargs != nullptr) {
            PyObject *key, *value;
            Py_ssize_t pos = 0;

            while (PyDict_Next(extraKwargs, &pos, &key, &value)) {
                self->map.emplace(convert(key), convert(value));
            }
            if (PyErr_Occurred()) return -1;
        }
        return 0;
    };

    try {
        if (map != nullptr) {
            self->map = map->map;
            return processExtraKwargs();
        } else if (processExtraKwargs() == -1) {
            return -1;
        }

        if (pyDict != nullptr) {
            PyObject *key, *value;
            Py_ssize_t pos = 0;

            while (PyDict_Next(pyDict, &pos, &key, &value)) {
                self->map.emplace(convert(key), convert(value));
            }
            if (PyErr_Occurred()) return -1;
            return 0;
        }

        if (pyMapping != nullptr) {
            auto keys = PyMapping_Keys(pyMapping);
            if (keys == nullptr) {
                return -1;
            }

            if (PyList_Check(keys) || PyTuple_Check(keys)) {
                // fast method
                auto fastKeys = PySequence_Fast(keys, "Shouldn't be happen (IntIntHashMap).");
                SAFE_DECREF(keys);
                if (fastKeys == nullptr) {
                    return -1;
                }

                const auto size = PySequence_Fast_GET_SIZE(fastKeys);
                auto items = PySequence_Fast_ITEMS(fastKeys);
                for (Py_ssize_t i = 0; i < size; ++i) {
                    auto key = items[i];
                    auto value = PyObject_GetItem(pyMapping, key);
                    if (value == nullptr) {
                        SAFE_DECREF(fastKeys);
                        return -1;
                    }

                    self->map.emplace(convert(key), convert(value));
                    SAFE_DECREF(value);
                }
                SAFE_DECREF(fastKeys);
            } else {
                const auto size = PyObject_Size(keys);
                for (Py_ssize_t i = 0; i < size; ++i) {
                    auto index = PyLong_FromSsize_t(i);
                    auto key = PyObject_GetItem(keys, index);
                    auto value = PyObject_GetItem(pyMapping, key);
                    self->map.emplace(convert(key), convert(value));
                    SAFE_DECREF(key);
                    SAFE_DECREF(value);
                    SAFE_DECREF(index);
                    SAFE_DECREF(keys);
                    if (PyErr_Occurred()) return -1;
                }
            }
            SAFE_DECREF(keys);
            return 0;
        }

        if (pyIterable != nullptr) {
            if (PyList_Check(pyIterable) || PyTuple_Check(pyIterable)) {
                auto fastIter = PySequence_Fast(pyIterable, "Shouldn't be happen (IntIntHashMap).");

                const auto size = PySequence_Fast_GET_SIZE(fastIter);
                auto items = PySequence_Fast_ITEMS(fastIter);
                for (Py_ssize_t i = 0; i < size; ++i) {
                    auto entry = items[i];

                    if (PyList_Check(entry)) {
                        if (PyList_GET_SIZE(entry) != 2) {
                            PyErr_SetString(PyExc_ValueError, "expected entry size == 2.");
                            return -1;
                        }

                        self->map.emplace(
                                convert(PyTuple_GET_ITEM(entry, 0)),
                                convert(PyTuple_GET_ITEM(entry, 1)));
                    } else if (PyTuple_Check(entry)) {
                        if (PyTuple_GET_SIZE(entry) != 2) {
                            PyErr_SetString(PyExc_ValueError, "expected entry size == 2.");
                            return -1;
                        }
                        self->map.emplace(
                                convert(PyTuple_GET_ITEM(entry, 0)),
                                convert(PyTuple_GET_ITEM(entry, 1)));
                    } else {
                        auto entryIter = PyObject_GetIter(entry);

                        auto key = PyIter_Next(entryIter);
                        auto value = PyIter_Next(entryIter);
                        if (key == nullptr || value == nullptr) {
                            PyErr_SetString(PyExc_ValueError, "expected entry size == 2.");
                            return -1;
                        }

                        self->map.emplace(convert(key), convert(value));
                        SAFE_DECREF(entryIter);
                        SAFE_DECREF(key);
                        SAFE_DECREF(value);
                    }

                    if (PyErr_Occurred()) {
                        SAFE_DECREF(fastIter);
                        return -1;
                    }
                }
                SAFE_DECREF(fastIter);
            } else {
                auto iter = PyObject_GetIter(pyIterable);
                if (iter == nullptr) {
                    PyErr_SetString(PyExc_TypeError, "Arg '__iterable' is not iterable.");
                    return -1;
                }

                PyObject *entry;
                while ((entry = PyIter_Next(iter)) != nullptr) {
                    if (PyList_Check(entry)) {
                        if (PyList_GET_SIZE(entry) != 2) {
                            PyErr_SetString(PyExc_ValueError, "expected entry size == 2.");
                            return -1;
                        }

                        self->map[PyLong_AsLong(PyList_GET_ITEM(entry, 0))] = PyLong_AsLong(PyList_GET_ITEM(entry, 1));
                    } else if (PyTuple_Check(entry)) {
                        if (PyTuple_GET_SIZE(entry) != 2) {
                            PyErr_SetString(PyExc_ValueError, "expected entry size == 2.");
                            return -1;
                        }

                        self->map.emplace(
                                convert(PyTuple_GET_ITEM(entry, 0)),
                                convert(PyTuple_GET_ITEM(entry, 1)));
                    } else {
                        auto entryIter = PyObject_GetIter(entry);

                        auto key = PyIter_Next(entryIter);
                        auto value = PyIter_Next(entryIter);
                        if (key == nullptr || value == nullptr) {
                            PyErr_SetString(PyExc_ValueError, "expected entry size == 2.");
                            return -1;
                        }

                        self->map.emplace(convert(key), convert(value));
                        SAFE_DECREF(entryIter);
                        SAFE_DECREF(key);
                        SAFE_DECREF(value);
                    }

                    if (PyErr_Occurred()) {
                        SAFE_DECREF(iter);
                        return -1;
                    }
                }
                SAFE_DECREF(iter);
            }
        }
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }

    return 0;
}

static void IntIntHashMap_dealloc(IntIntHashMap *self) {
    self->map.~table();
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *IntIntHashMap_copy(PyObject *pySelf) {
    auto *self = reinterpret_cast<IntIntHashMap *>(pySelf);

    auto *copy = Py_CreateObj<IntIntHashMap>(IntIntHashMapType);
    if (copy == nullptr) return PyErr_NoMemory();

    try {
        copy->map = self->map;
    } catch (const std::exception &e) {
        PyObject_Del(copy);
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }

    return reinterpret_cast<PyObject *>(copy);
}

#ifdef IS_PYTHON_39_OR_LATER
static PyObject *IntIntHashMap_class_getitem(PyObject *cls, PyObject *item) {
    return Py_GenericAlias(cls, item);
}
#endif

static PyMethodDef IntIntHashMap_methods[] = {
        {"copy", (PyCFunction) IntIntHashMap_copy, METH_NOARGS},
#ifdef IS_PYTHON_39_OR_LATER
        {"__class_getitem__", (PyCFunction) IntIntHashMap_class_getitem, METH_O | METH_CLASS},
#endif
        {nullptr}
};

static struct PyModuleDef IntIntHashMap_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.IntIntHashMap",
        "An IntIntHashMap_module that creates an IntIntHashMap",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

void initializeIntIntHashMapType(PyTypeObject &type) {
    type.tp_name = "IntIntHashMap";
    type.tp_basicsize = sizeof(IntIntHashMap);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_methods = IntIntHashMap_methods;
    type.tp_init = (initproc) IntIntHashMap_init;
    type.tp_new = PyType_GenericNew;
    type.tp_dealloc = (destructor) IntIntHashMap_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
    type.tp_hash = PyObject_HashNotImplemented;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_IntIntHashMap() {
    initializeIntIntHashMapType(IntIntHashMapType);

    PyObject *object = PyModule_Create(&IntIntHashMap_module);
    if (object == nullptr)
        return nullptr;

    Py_INCREF(&IntIntHashMapType);
    if (PyModule_AddObject(object, "IntIntHashMap", (PyObject *) &IntIntHashMapType) < 0) {
        Py_DECREF(&IntIntHashMapType);
        Py_DECREF(object);
        return nullptr;
    }

    return object;
}
#pragma clang diagnostic pop

}
