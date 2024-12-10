//
// Created by xia__mc on 2024/12/8.
//

#include "SIMD.h"
#include "utils/simd/SIMDHelper.h"
#include "utils/simd/SIMDUtils.h"
#include "utils/simd/BitonicSort.h"
#include "utils/PythonUtils.h"
#include "utils/include/qreverse.hpp"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-macro-parentheses"

#define DEFINE_SIMD_FUNCTION(type, name) \
    static PyObject *SIMD_memcpy##name([[maybe_unused]] PyObject *self, \
                                        PyObject *const *args, Py_ssize_t nargs) { \
        return SIMD_memcpy<type>(args, nargs); \
    } \
    static PyObject *SIMD_memcpy##name##Aligned([[maybe_unused]] PyObject *self, \
                                                PyObject *const *args, Py_ssize_t nargs) { \
        return SIMD_memcpyAligned<type>(args, nargs); \
    } \
    static PyObject *SIMD_reverse##name([[maybe_unused]] PyObject *self, \
                                        PyObject *const *args, Py_ssize_t nargs) { \
        return SIMD_reverse<type>(args, nargs); \
    }
#pragma clang diagnostic pop

#define REGISTER_SIMD_FUNCTION(name) \
    {"memcpy" #name, (PyCFunction) SIMD_memcpy##name, METH_FASTCALL, nullptr}, \
    {"memcpy" #name "Aligned", (PyCFunction) SIMD_memcpy##name##Aligned, METH_FASTCALL, nullptr}, \
    {"reverse" #name, (PyCFunction) SIMD_reverse##name, METH_FASTCALL, nullptr}

template<typename T>
static __forceinline PyObject *SIMD_memcpy(PyObject *const *args, Py_ssize_t nargs) {
    // Ensure we have exactly 3 arguments
    if (nargs != 3) {
        PyErr_SetString(PyExc_TypeError,
                        "Function takes exactly 3 arguments (__addressFrom, __addressTo, __count)");
        return nullptr;
    }

    // Extract arguments
    auto *addressFrom = reinterpret_cast<T *>(PyLong_AsUnsignedLongLong(args[0]));
    auto *addressTo = reinterpret_cast<T *>(PyLong_AsUnsignedLongLong(args[1]));
    size_t count = PyLong_AsSize_t(args[2]);

    // Perform the memory copy
    simd::simdMemCpy(addressFrom, addressTo, count);

    Py_RETURN_NONE;
}

template<typename T>
static __forceinline PyObject *SIMD_memcpyAligned(PyObject *const *args, Py_ssize_t nargs) {
    // Ensure we have exactly 3 arguments
    if (nargs != 3) {
        PyErr_SetString(PyExc_TypeError,
                        "Function takes exactly 3 arguments (__addressFrom, __addressTo, __count)");
        return nullptr;
    }

    // Extract arguments
    auto *addressFrom = reinterpret_cast<T *>(PyLong_AsVoidPtr(args[0]));
    auto *addressTo = reinterpret_cast<T *>(PyLong_AsVoidPtr(args[1]));
    size_t count = PyLong_AsSize_t(args[2]);

    // Perform the memory copy
    simd::simdMemCpyAligned(addressFrom, addressTo, count);

    Py_RETURN_NONE;
}

template<typename T>
static __forceinline PyObject *SIMD_reverse(PyObject *const *args, Py_ssize_t nargs) {
    // Ensure we have exactly 3 arguments
    if (nargs != 2) {
        PyErr_SetString(PyExc_TypeError, "Function takes exactly 2 arguments (__address, __count)");
        return nullptr;
    }

    // Extract arguments
    auto *address = reinterpret_cast<T *>(PyLong_AsVoidPtr(args[0]));
    size_t count = PyLong_AsSize_t(args[1]);

    // Perform the memory copy
    simd::simdReverse(address, count);

    Py_RETURN_NONE;
}

extern "C" {

static PyTypeObject SIMDType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
};

static int SIMD_init([[maybe_unused]] SIMD *self,
                     [[maybe_unused]] PyObject *args, [[maybe_unused]] PyObject *kwargs) {
    return 0;
}

static void SIMD_dealloc(SIMD *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *SIMD_enter(PyObject *self, [[maybe_unused]] PyObject *args) {
    Py_INCREF(self);
    return self;
}

static PyObject *SIMD_exit([[maybe_unused]] PyObject *self,
                           [[maybe_unused]] PyObject *const *args, [[maybe_unused]] Py_ssize_t nargs) {
    Py_RETURN_NONE;
}

static PyObject *SIMD_isSSE41Supported([[maybe_unused]] PyObject *pySelf) noexcept {
    Py_RETURN_BOOL(simd::IS_SSE41_SUPPORTED)
}

static PyObject *SIMD_isAVX2Supported([[maybe_unused]] PyObject *pySelf) noexcept {
    Py_RETURN_BOOL(simd::IS_AVX2_SUPPORTED)
}

static PyObject *SIMD_isAVX512Supported([[maybe_unused]] PyObject *pySelf) noexcept {
    Py_RETURN_BOOL(simd::IS_AVX512_SUPPORTED)
}

static PyObject *SIMD_isSSSE3Supported([[maybe_unused]] PyObject *pySelf) noexcept {
    Py_RETURN_BOOL(simd::IS_SSSE3_SUPPORTED)
}

static PyObject *SIMD_isArmNeonSupported([[maybe_unused]] PyObject *pySelf) noexcept {
    Py_RETURN_BOOL(simd::IS_ARM_NEON_SUPPORTED)
}

static PyObject *SIMD_setAVX512Vector32([[maybe_unused]] PyObject *pySelf,
                                        PyObject *const *args, Py_ssize_t nargs) noexcept {
#if !defined(__arm__) && !defined(__arm64__)
    if (nargs != 17) {
        PyErr_SetString(PyExc_TypeError, "Function takes exactly 17 arguments (__ptr, ...)");
        return nullptr;
    }

    (*((__m512i *) PyLong_AsVoidPtr(args[0]))) = _mm512_set_epi32(
            PyFast_AsInt(args[1]), PyFast_AsInt(args[2]), PyFast_AsInt(args[3]), PyFast_AsInt(args[4]),
            PyFast_AsInt(args[5]), PyFast_AsInt(args[6]), PyFast_AsInt(args[7]), PyFast_AsInt(args[8]),
            PyFast_AsInt(args[9]), PyFast_AsInt(args[10]), PyFast_AsInt(args[11]), PyFast_AsInt(args[12]),
            PyFast_AsInt(args[13]), PyFast_AsInt(args[14]), PyFast_AsInt(args[15]), PyFast_AsInt(args[16])
    );

    Py_RETURN_NONE;
#else
    PyErr_SetString(PyExc_NotImplementedError, "AVX-512 is not supported on this architecture.");
    return nullptr;
#endif
}

static PyObject *SIMD_setAVX512Vector16([[maybe_unused]] PyObject *pySelf,
                                        PyObject *const *args, Py_ssize_t nargs) noexcept {
#if !defined(__arm__) && !defined(__arm64__)
    if (nargs != 33) {
        PyErr_SetString(PyExc_TypeError, "Function takes exactly 33 arguments (__ptr, ...)");
        return nullptr;
    }

    (*((__m512i *) PyLong_AsVoidPtr(args[0]))) = _mm512_set_epi16(
            PyFast_AsShort(args[1]), PyFast_AsShort(args[2]), PyFast_AsShort(args[3]), PyFast_AsShort(args[4]),
            PyFast_AsShort(args[5]), PyFast_AsShort(args[6]), PyFast_AsShort(args[7]), PyFast_AsShort(args[8]),
            PyFast_AsShort(args[9]), PyFast_AsShort(args[10]), PyFast_AsShort(args[11]), PyFast_AsShort(args[12]),
            PyFast_AsShort(args[13]), PyFast_AsShort(args[14]), PyFast_AsShort(args[15]), PyFast_AsShort(args[16]),
            PyFast_AsShort(args[17]), PyFast_AsShort(args[18]), PyFast_AsShort(args[19]), PyFast_AsShort(args[20]),
            PyFast_AsShort(args[21]), PyFast_AsShort(args[22]), PyFast_AsShort(args[23]), PyFast_AsShort(args[24]),
            PyFast_AsShort(args[25]), PyFast_AsShort(args[26]), PyFast_AsShort(args[27]), PyFast_AsShort(args[28]),
            PyFast_AsShort(args[29]), PyFast_AsShort(args[30]), PyFast_AsShort(args[31]), PyFast_AsShort(args[32])
    );

    Py_RETURN_NONE;
#else
    PyErr_SetString(PyExc_NotImplementedError, "AVX-512 is not supported on this architecture.");
    return nullptr;
#endif
}

static PyObject *SIMD_setAVX512Vector8([[maybe_unused]] PyObject *pySelf,
                                       PyObject *const *args, Py_ssize_t nargs) noexcept {
#if !defined(__arm__) && !defined(__arm64__)
    if (nargs != 65) {
        PyErr_SetString(PyExc_TypeError, "Function takes exactly 65 arguments (__ptr, ...)");
        return nullptr;
    }

    (*((__m512i *) PyLong_AsVoidPtr(args[0]))) = _mm512_set_epi8(
            PyFast_AsChar(args[1]), PyFast_AsChar(args[2]), PyFast_AsChar(args[3]), PyFast_AsChar(args[4]),
            PyFast_AsChar(args[5]), PyFast_AsChar(args[6]), PyFast_AsChar(args[7]), PyFast_AsChar(args[8]),
            PyFast_AsChar(args[9]), PyFast_AsChar(args[10]), PyFast_AsChar(args[11]), PyFast_AsChar(args[12]),
            PyFast_AsChar(args[13]), PyFast_AsChar(args[14]), PyFast_AsChar(args[15]), PyFast_AsChar(args[16]),
            PyFast_AsChar(args[17]), PyFast_AsChar(args[18]), PyFast_AsChar(args[19]), PyFast_AsChar(args[20]),
            PyFast_AsChar(args[21]), PyFast_AsChar(args[22]), PyFast_AsChar(args[23]), PyFast_AsChar(args[24]),
            PyFast_AsChar(args[25]), PyFast_AsChar(args[26]), PyFast_AsChar(args[27]), PyFast_AsChar(args[28]),
            PyFast_AsChar(args[29]), PyFast_AsChar(args[30]), PyFast_AsChar(args[31]), PyFast_AsChar(args[32]),
            PyFast_AsChar(args[33]), PyFast_AsChar(args[34]), PyFast_AsChar(args[35]), PyFast_AsChar(args[36]),
            PyFast_AsChar(args[37]), PyFast_AsChar(args[38]), PyFast_AsChar(args[39]), PyFast_AsChar(args[40]),
            PyFast_AsChar(args[41]), PyFast_AsChar(args[42]), PyFast_AsChar(args[43]), PyFast_AsChar(args[44]),
            PyFast_AsChar(args[45]), PyFast_AsChar(args[46]), PyFast_AsChar(args[47]), PyFast_AsChar(args[48]),
            PyFast_AsChar(args[49]), PyFast_AsChar(args[50]), PyFast_AsChar(args[51]), PyFast_AsChar(args[52]),
            PyFast_AsChar(args[53]), PyFast_AsChar(args[54]), PyFast_AsChar(args[55]), PyFast_AsChar(args[56]),
            PyFast_AsChar(args[57]), PyFast_AsChar(args[58]), PyFast_AsChar(args[59]), PyFast_AsChar(args[60]),
            PyFast_AsChar(args[61]), PyFast_AsChar(args[62]), PyFast_AsChar(args[63]), PyFast_AsChar(args[64])
    );

    Py_RETURN_NONE;
#else
    PyErr_SetString(PyExc_NotImplementedError, "AVX-512 is not supported on this architecture.");
    return nullptr;
#endif
}

static PyObject *SIMD_setAVX2Vector32([[maybe_unused]] PyObject *pySelf,
                                      PyObject *const *args, Py_ssize_t nargs) noexcept {
#if !defined(__arm__) && !defined(__arm64__)
    if (nargs != 9) {
        PyErr_SetString(PyExc_TypeError, "Function takes exactly 9 arguments (__ptr, ...)");
        return nullptr;
    }

    (*((__m256i *) PyLong_AsVoidPtr(args[0]))) = _mm256_set_epi32(
            PyFast_AsInt(args[1]), PyFast_AsInt(args[2]), PyFast_AsInt(args[3]), PyFast_AsInt(args[4]),
            PyFast_AsInt(args[5]), PyFast_AsInt(args[6]), PyFast_AsInt(args[7]), PyFast_AsInt(args[8])
    );

    Py_RETURN_NONE;
#else
    PyErr_SetString(PyExc_NotImplementedError, "AVX-2 is not supported on this architecture.");
    return nullptr;
#endif
}

static PyObject *SIMD_setAVX2Vector16([[maybe_unused]] PyObject *pySelf,
                                      PyObject *const *args, Py_ssize_t nargs) noexcept {
#if !defined(__arm__) && !defined(__arm64__)
    if (nargs != 17) {
        PyErr_SetString(PyExc_TypeError, "Function takes exactly 17 arguments (__ptr, ...)");
        return nullptr;
    }

    (*((__m256i *) PyLong_AsVoidPtr(args[0]))) = _mm256_set_epi16(
            PyFast_AsShort(args[1]), PyFast_AsShort(args[2]), PyFast_AsShort(args[3]), PyFast_AsShort(args[4]),
            PyFast_AsShort(args[5]), PyFast_AsShort(args[6]), PyFast_AsShort(args[7]), PyFast_AsShort(args[8]),
            PyFast_AsShort(args[9]), PyFast_AsShort(args[10]), PyFast_AsShort(args[11]), PyFast_AsShort(args[12]),
            PyFast_AsShort(args[13]), PyFast_AsShort(args[14]), PyFast_AsShort(args[15]), PyFast_AsShort(args[16])
    );

    Py_RETURN_NONE;
#else
    PyErr_SetString(PyExc_NotImplementedError, "AVX-2 is not supported on this architecture.");
    return nullptr;
#endif
}

static PyObject *SIMD_setAVX2Vector8([[maybe_unused]] PyObject *pySelf,
                                     PyObject *const *args, Py_ssize_t nargs) noexcept {
#if !defined(__arm__) && !defined(__arm64__)
    if (nargs != 33) {
        PyErr_SetString(PyExc_TypeError, "Function takes exactly 33 arguments (__ptr, ...)");
        return nullptr;
    }

    (*((__m256i *) PyLong_AsVoidPtr(args[0]))) = _mm256_set_epi8(
            PyFast_AsChar(args[1]), PyFast_AsChar(args[2]), PyFast_AsChar(args[3]), PyFast_AsChar(args[4]),
            PyFast_AsChar(args[5]), PyFast_AsChar(args[6]), PyFast_AsChar(args[7]), PyFast_AsChar(args[8]),
            PyFast_AsChar(args[9]), PyFast_AsChar(args[10]), PyFast_AsChar(args[11]), PyFast_AsChar(args[12]),
            PyFast_AsChar(args[13]), PyFast_AsChar(args[14]), PyFast_AsChar(args[15]), PyFast_AsChar(args[16]),
            PyFast_AsChar(args[17]), PyFast_AsChar(args[18]), PyFast_AsChar(args[19]), PyFast_AsChar(args[20]),
            PyFast_AsChar(args[21]), PyFast_AsChar(args[22]), PyFast_AsChar(args[23]), PyFast_AsChar(args[24]),
            PyFast_AsChar(args[25]), PyFast_AsChar(args[26]), PyFast_AsChar(args[27]), PyFast_AsChar(args[28]),
            PyFast_AsChar(args[29]), PyFast_AsChar(args[30]), PyFast_AsChar(args[31]), PyFast_AsChar(args[32])
    );

    Py_RETURN_NONE;
#else
    PyErr_SetString(PyExc_NotImplementedError, "AVX-2 is not supported on this architecture.");
    return nullptr;
#endif
}

// util functions
DEFINE_SIMD_FUNCTION(int, Int)
DEFINE_SIMD_FUNCTION(unsigned int, UnsignedInt)
DEFINE_SIMD_FUNCTION(long, Long)
DEFINE_SIMD_FUNCTION(unsigned long, UnsignedLong)
DEFINE_SIMD_FUNCTION(long long, LongLong)
DEFINE_SIMD_FUNCTION(unsigned long long, UnsignedLongLong)
DEFINE_SIMD_FUNCTION(short, Short)
DEFINE_SIMD_FUNCTION(unsigned short, UnsignedShort)

DEFINE_SIMD_FUNCTION(float, Float)
DEFINE_SIMD_FUNCTION(double, Double)
DEFINE_SIMD_FUNCTION(long double, LongDouble)

DEFINE_SIMD_FUNCTION(char, Char)
DEFINE_SIMD_FUNCTION(unsigned char, UnsignedChar)
DEFINE_SIMD_FUNCTION(wchar_t, WChar)
DEFINE_SIMD_FUNCTION(char16_t, Char16)
DEFINE_SIMD_FUNCTION(char32_t, Char32)

DEFINE_SIMD_FUNCTION(bool, Bool)

DEFINE_SIMD_FUNCTION(int8_t, Int8)
DEFINE_SIMD_FUNCTION(uint8_t, UInt8)
DEFINE_SIMD_FUNCTION(int16_t, Int16)
DEFINE_SIMD_FUNCTION(uint16_t, UInt16)
DEFINE_SIMD_FUNCTION(int32_t, Int32)
DEFINE_SIMD_FUNCTION(uint32_t, UInt32)
DEFINE_SIMD_FUNCTION(int64_t, Int64)
DEFINE_SIMD_FUNCTION(uint64_t, UInt64)

DEFINE_SIMD_FUNCTION(void*, VoidPtr)
DEFINE_SIMD_FUNCTION(int*, IntPtr)
DEFINE_SIMD_FUNCTION(float*, FloatPtr)

DEFINE_SIMD_FUNCTION(PyObject *, PyObjectPtr)

static PyMethodDef SIMD_methods[] = {
        {"__enter__", (PyCFunction) SIMD_enter, METH_NOARGS, nullptr},
        {"__exit__", (PyCFunction) SIMD_exit, METH_FASTCALL, nullptr},
        {"isSSE41Supported", (PyCFunction) SIMD_isSSE41Supported, METH_NOARGS, nullptr},
        {"isAVX2Supported", (PyCFunction) SIMD_isAVX2Supported, METH_NOARGS, nullptr},
        {"isAVX512Supported", (PyCFunction) SIMD_isAVX512Supported, METH_NOARGS, nullptr},
        {"isSSSE3Supported", (PyCFunction) SIMD_isSSSE3Supported, METH_NOARGS, nullptr},
        {"isArmNeonSupported", (PyCFunction) SIMD_isArmNeonSupported, METH_NOARGS, nullptr},
        {"setAVX512Vector32", (PyCFunction) SIMD_setAVX512Vector32, METH_FASTCALL, nullptr},
        {"setAVX512Vector16", (PyCFunction) SIMD_setAVX512Vector16, METH_FASTCALL, nullptr},
        {"setAVX512Vector8", (PyCFunction) SIMD_setAVX512Vector8, METH_FASTCALL, nullptr},
        {"setAVX2Vector32", (PyCFunction) SIMD_setAVX2Vector32, METH_FASTCALL, nullptr},
        {"setAVX2Vector16", (PyCFunction) SIMD_setAVX2Vector16, METH_FASTCALL, nullptr},
        {"setAVX2Vector8", (PyCFunction) SIMD_setAVX2Vector8, METH_FASTCALL, nullptr},

        // util functions
        REGISTER_SIMD_FUNCTION(Int),
        REGISTER_SIMD_FUNCTION(UnsignedInt),
        REGISTER_SIMD_FUNCTION(Long),
        REGISTER_SIMD_FUNCTION(UnsignedLong),
        REGISTER_SIMD_FUNCTION(LongLong),
        REGISTER_SIMD_FUNCTION(UnsignedLongLong),
        REGISTER_SIMD_FUNCTION(Short),
        REGISTER_SIMD_FUNCTION(UnsignedShort),

        REGISTER_SIMD_FUNCTION(Float),
        REGISTER_SIMD_FUNCTION(Double),
        REGISTER_SIMD_FUNCTION(LongDouble),

        REGISTER_SIMD_FUNCTION(Char),
        REGISTER_SIMD_FUNCTION(UnsignedChar),
        REGISTER_SIMD_FUNCTION(WChar),
        REGISTER_SIMD_FUNCTION(Char16),
        REGISTER_SIMD_FUNCTION(Char32),

        REGISTER_SIMD_FUNCTION(Bool),

        REGISTER_SIMD_FUNCTION(Int8),
        REGISTER_SIMD_FUNCTION(UInt8),
        REGISTER_SIMD_FUNCTION(Int16),
        REGISTER_SIMD_FUNCTION(UInt16),
        REGISTER_SIMD_FUNCTION(Int32),
        REGISTER_SIMD_FUNCTION(UInt32),
        REGISTER_SIMD_FUNCTION(Int64),
        REGISTER_SIMD_FUNCTION(UInt64),

        REGISTER_SIMD_FUNCTION(VoidPtr),
        REGISTER_SIMD_FUNCTION(IntPtr),
        REGISTER_SIMD_FUNCTION(FloatPtr),

        REGISTER_SIMD_FUNCTION(PyObjectPtr),
        {nullptr, nullptr, 0, nullptr}
};


void initializeSIMDType(PyTypeObject &type) {
    type.tp_name = "SIMD";
    type.tp_basicsize = sizeof(SIMD);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_methods = SIMD_methods;
    type.tp_init = (initproc) SIMD_init;
    type.tp_new = PyType_GenericNew;
    type.tp_dealloc = (destructor) SIMD_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
}

static struct PyModuleDef SIMD_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.SIMD",
        "Allow access to C SIMD APIs.",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_SIMD() {
    initializeSIMDType(SIMDType);

    PyObject *object = PyModule_Create(&SIMD_module);
    if (object == nullptr)
        return nullptr;

    Py_INCREF(&SIMDType);
    if (PyModule_AddObject(object, "SIMD", (PyObject *) &SIMDType) < 0) {
        Py_DECREF(&SIMDType);
        Py_DECREF(object);
        return nullptr;
    }

    return object;
}
#pragma clang diagnostic pop

}
