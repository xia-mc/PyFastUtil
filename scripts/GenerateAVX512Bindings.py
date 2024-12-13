import os
from dataclasses import dataclass

from pyfastutil.objects import ObjectArrayList

RESULT_FILE = "SIMDLowAVX512.cpp"
RESULT_PYI_FILE = "SIMDLowAVX512.pyi"

BYPASS_FUNC = ("_kunpackb_mask16",)  # IDK why, maybe msvc issue

FORCE_CONSTANT = {
    "_kshiftli_mask16": (1,),
    "_kshiftri_mask16": (1,)
}

# arg id (count from zero), immediate min, immediate max
# some functions' metadata are generated by chatgpt
IMMEDIATE_MAP = {
    "_mm_getexp_round_ss": [
        (2, 0, 4)
    ],
    "_mm_mask_getexp_round_ss": [
        (4, 0, 4)
    ],
    "_mm_maskz_getexp_round_ss": [
        (3, 0, 4)
    ],
    "_mm_getexp_round_sd": [
        (2, 0, 4)
    ],
    "_mm_mask_getexp_round_sd": [
        (4, 0, 4)
    ],
    "_mm_maskz_getexp_round_sd": [
        (3, 0, 4)
    ],
    "_mm512_shuffle_epi32": [
        (1, 0, 255)
    ],
    "_mm512_mask_shuffle_epi32": [
        (3, 0, 255)
    ],
    "_mm512_maskz_shuffle_epi32": [
        (2, 0, 255)
    ],
    "_mm512_getmant_round_pd": [
        (1, 0, 3),
        (2, 0, 2),
        (3, 0, 4)
    ],
    "_mm512_mask_getmant_round_pd": [
        (3, 0, 3),
        (4, 0, 2),
        (5, 0, 4)
    ],
    "_mm512_maskz_getmant_round_pd": [
        (2, 0, 3),
        (3, 0, 2),
        (4, 0, 4)
    ],
    "_mm512_getmant_round_ps": [
        (1, 0, 3),
        (2, 0, 2),
        (3, 0, 4)
    ],
    "_mm512_mask_getmant_round_ps": [
        (3, 0, 3),
        (4, 0, 2),
        (5, 0, 4)
    ],
    "_mm512_maskz_getmant_round_ps": [
        (2, 0, 3),
        (3, 0, 2),
        (4, 0, 4)
    ],
    "_mm_getmant_round_sd": [
        (2, 0, 3),
        (3, 0, 2),
        (4, 0, 4)
    ],
    "_mm512_permutexvar_epi32": [
        (2, 0, 255)
    ],
    "_mm512_maskz_permutexvar_epi32": [
        (3, 0, 255)
    ],
    "_mm512_slli_epi32": [
        (1, 0, 31)
    ],
    "_mm512_srli_epi32": [
        (1, 0, 31)
    ],
    "_mm512_maskz_shuffle_ps": [
        (3, 0, 255)
    ],
    "_mm512_ror_epi32": [
        (1, 0, 31)
    ],
    "_mm512_maskz_ror_epi32": [
        (2, 0, 31)
    ],
    "_mm512_insert_epi32": [
        (2, 0, 15)
    ],
    "_mm512_add_round_pd": [
        (2, 0, 4)
    ],
    "_mm512_sub_round_ps": [
        (2, 0, 4)
    ],
    "_mm512_mul_round_ps": [
        (2, 0, 4)
    ],
    "_mm512_div_round_pd": [
        (2, 0, 4)
    ],
    "_mm512_getexp_round_pd": [
        (1, 0, 3),
        (2, 0, 2),
        (3, 0, 4)
    ],
    "_mm512_getexp_round_ps": [
        (1, 0, 3),
        (2, 0, 2),
        (3, 0, 4)
    ]
}

# noinspection LongLine
FUNC_TEMPLATE = """
static __forceinline PyObject *SIMDLowAVX512_{function_name}_impl([[maybe_unused]] PyObject *const *args, [[maybe_unused]] Py_ssize_t nargs) noexcept {
#if !defined(__arm__) && !defined(__arm64__)
    if (nargs != {num_args}) {
        PyErr_SetString(PyExc_TypeError, "Function takes exactly {num_args} arguments.");
        return nullptr;
    }

{arg_parsing}

{operation}

    Py_RETURN_NONE;
#else
    PyErr_SetString(PyExc_NotImplementedError, "AVX-512 is not supported on this architecture.");
    return nullptr;
#endif
}
"""

# noinspection LongLine
FUNC_NOT_IMPL_TEMPLATE = """
static __forceinline PyObject *SIMDLowAVX512_{function_name}_impl([[maybe_unused]] PyObject *const *args, [[maybe_unused]] Py_ssize_t nargs) noexcept {
#if !defined(__arm__) && !defined(__arm64__)
    PyErr_SetString(PyExc_NotImplementedError, "Target C Method require immediate numbers, and this method is not supported in PyFastUtil now.");
    return nullptr;
#else
    PyErr_SetString(PyExc_NotImplementedError, "AVX-512 is not supported on this architecture.");
    return nullptr;
#endif
}
"""

# noinspection LongLine
C_FUNC_TEMPLATE = """
static PyObject *SIMDLowAVX512_{function_name}([[maybe_unused]] PyObject *pySelf, PyObject *const *args, Py_ssize_t nargs) noexcept {
    return SIMDLowAVX512_{function_name}_impl(args, nargs);
}
"""

REGISTER_TEMPLATE = """    {"{function_name}", (PyCFunction) SIMDLowAVX512_{function_name}, METH_FASTCALL, nullptr},
"""

# noinspection LongLine
FILE_TEMPLATE = """//
// Created by xia__mc on 2024/12/9.
//

#include "SIMDLowAVX512.h"
#include "utils/simd/SIMDHelper.h"
#include "utils/simd/SIMDUtils.h"
#include "utils/PythonUtils.h"

{function_def}

extern "C" {

static PyTypeObject SIMDLowAVX512Type = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
};

static int SIMDLowAVX512_init([[maybe_unused]] SIMDLowAVX512 *self,
                              [[maybe_unused]] PyObject *args, [[maybe_unused]] PyObject *kwargs) {
    return 0;
}

static void SIMDLowAVX512_dealloc(SIMDLowAVX512 *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *SIMDLowAVX512_enter(PyObject *self, [[maybe_unused]] PyObject *args) {
    Py_INCREF(self);
    return self;
}

static PyObject *SIMDLowAVX512_exit([[maybe_unused]] PyObject *self,
                                    [[maybe_unused]] PyObject *const *args, [[maybe_unused]] Py_ssize_t nargs) {
    Py_RETURN_NONE;
}

{c_function_def}

static PyMethodDef SIMDLowAVX512_methods[] = {
        {"__enter__", (PyCFunction) SIMDLowAVX512_enter, METH_NOARGS, nullptr},
        {"__exit__", (PyCFunction) SIMDLowAVX512_exit, METH_FASTCALL, nullptr},
{function_register}
};

void initializeSIMDLowAVX512Type(PyTypeObject &type) {
    type.tp_name = "SIMDLowAVX512";
    type.tp_basicsize = sizeof(SIMDLowAVX512);
    type.tp_itemsize = 0;
    type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    type.tp_methods = SIMDLowAVX512_methods;
    type.tp_init = (initproc) SIMDLowAVX512_init;
    type.tp_new = PyType_GenericNew;
    type.tp_dealloc = (destructor) SIMDLowAVX512_dealloc;
    type.tp_alloc = PyType_GenericAlloc;
    type.tp_free = PyObject_Del;
}

static struct PyModuleDef SIMDLowAVX512_module = {
        PyModuleDef_HEAD_INIT,
        "__pyfastutil.SIMDLowAVX512",
        "Allow access to raw C AVX512 APIs.",
        -1,
        nullptr, nullptr, nullptr, nullptr, nullptr
};

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_SIMDLowAVX512() {
    initializeSIMDLowAVX512Type(SIMDLowAVX512Type);

    PyObject *object = PyModule_Create(&SIMDLowAVX512_module);
    if (object == nullptr)
        return nullptr;

    Py_INCREF(&SIMDLowAVX512Type);
    if (PyModule_AddObject(object, "SIMDLowAVX512", (PyObject *) &SIMDLowAVX512Type) < 0) {
        Py_DECREF(&SIMDLowAVX512Type);
        Py_DECREF(object);
        return nullptr;
    }

    return object;
}
#pragma clang diagnostic pop
}
"""

PYI_TEMPLATE = """
from typing import TypeVar, NoReturn

from .__SIMDLowAVX512Defines import *

Ptr = TypeVar("Ptr", bound=int)
NULL: Ptr

RaisesNotImplementedError = TypeVar("RaisesNotImplementedError", bound=NoReturn)
\"\"\"
A marker type to indicate that a function raises a NotImplementedError.

This type is used to explicitly document functions that are not currently
supported in PyFastUtil and will raise a NotImplementedError when called.
\"\"\"

class SIMDLowAVX512:
    \"\"\"
    A class for performing AVX-512 SIMD operations.

    This class provides Python bindings for low-level AVX-512 SIMD instructions. It is automatically
    generated and designed for advanced users who need direct access to AVX-512 operations. All
    methods in this class map directly to C methods and are intended to be used with aligned memory
    pointers.

    **Key Features**:
        - Supports operations on AVX-512 vectors via 64-byte aligned pointers.
        - Methods directly map to C AVX-512 intrinsics with minimal abstraction.
        - Best used in conjunction with the `Unsafe` class for memory management.

    **Best Practice**:
        Use this class within a `with` statement to ensure that SIMD operations are scoped and
        memory management is clear. For example:

        ```python
        with Unsafe() as unsafe, SIMDLowAVX512() as simd:
            vec512i: Ptr = unsafe.aligned_malloc(64, 64)
            simd._mm512_set_epi64(vec512i, 1, 2, 3, 4, 5, 6, 7, 8)
            // do some operations ...
            unsafe.aligned_free(vec512i)
            vec512i = NULL
        ```

    **Implementation Details**:
        - This class is automatically generated by a script.
        - Non-POD (Plain Old Data) types are mapped via pointers.
        - For C methods with non-void return types, the return value is mapped to the first argument
          `__result`, which must be a pointer to memory with the appropriate alignment.

    **Warnings**:
        - All pointers passed to this class must be 64-byte aligned.
        - Improper use of these methods can lead to memory corruption, crashes, or undefined behavior.
    \"\"\"

    def __init__(self) -> None:
        \"\"\"
        Initializes the SIMDLowAVX512 context.

        This constructor does not perform any AVX-512 operations. It prepares the object to be
        used in a `with` statement for scoping SIMD operations.
        \"\"\"
        pass

    def __enter__(self) -> SIMDLowAVX512: ...
    
    def __exit__(self, exc_type, exc_val, exc_tb) -> None: ...

{function_def}
"""

PYI_FUNC_TEMPLATE = """    def {function_name}(self, {function_args}) -> None:
        \"\"\"
        Executes the {function_name} operation.

        C method prototype:
            {prototype}

        This method performs the corresponding AVX-512 operation. Ensure that 
        all arguments meet the required constraints as specified in the C 
        method prototype.
        \"\"\"
        pass
"""

PYI_FUNC_NOT_IMPL_TEMPLATE = """    def {function_name}(self, {function_args}) -> RaisesNotImplementedError:
        \"\"\"
        {function_name} is not supported in PyFastUtil.

        C method prototype:
            {prototype}

        This method is not implemented because it relies on features that are 
        not currently supported in PyFastUtil. Specifically, it requires 
        immediate numbers, which cannot be dynamically passed in Python.

        Attempting to call this method will raise a NotImplementedError.
        \"\"\"
        pass
"""


@dataclass
class SIMDFunc:
    name: str
    returnType: str
    argTypes: tuple[str, ...]
    argNames: tuple[str, ...]
    unsupported: bool = False


def getFunctions(codes: str) -> list[SIMDFunc]:
    """dict[function name, function def]"""
    functions: list[tuple[str, str]] = ObjectArrayList()  # lastLine, code
    """
    lastLine example: extern __inline __m512d
    code example: _mm512_mask_permutex2var_pd (__m512d __A, __mmask8 __U, __m512i __I,
                                            __m512d __B)
    {
        return (__m512d) __builtin_ia32_vpermt2varpd512_mask ((__v8di) __I
                                /* idx */ ,
                                (__v8df) __A,
                                (__v8df) __B,
                                (__mmask8) __U);
    }
    """

    start = False
    code: str = ""
    lastLine: str = ""
    lastLine2: str = ""
    for line in codes.splitlines():
        if (line == "__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))"
                or line == "__attribute__((__gnu_inline__, __always_inline__, __artificial__))"):
            start = True
            if code != "":
                functions.append((lastLine2, code))
                code = ""
            continue
        elif start:
            code += line + "\n"
        if line.startswith("extern"):
            lastLine2 = lastLine
            lastLine = line

    result = ObjectArrayList(len(functions))
    for function in functions:
        name = function[1].split("(", 1)[0]
        while name.endswith(" "):
            name = name.removesuffix(" ")

        if name in BYPASS_FUNC:
            continue

        returnType = function[0].removeprefix("extern __inline")
        while returnType.endswith(" "):
            returnType = returnType.removesuffix(" ")
        while returnType.startswith(" "):
            returnType = returnType.removeprefix(" ")

        args: list[str] = (
            function[1]
            .split("(", 1)[1]
            .split(")\n", 1)[0]
            .replace("\n", "")
            .replace("\t", "")
            .replace(", ", ",")  # __m512d __A,__mmask8 __U,__m512i __I,__m512d __B
            .split(",")
        )

        argTypes = ObjectArrayList()
        argNames = ObjectArrayList()
        for arg in args:
            argType = ""
            delayChars = ""
            for c in arg.rsplit(" __", 1)[0]:
                if c == " ":
                    delayChars += " "
                else:
                    if argType != "":
                        argType += delayChars
                    argType += c
                    delayChars = ""
            if argType != "void":
                # special if
                if "*__" in argType:
                    argType = argType.split("*__")[0] + "*"

                argTypes.append(argType)

                argName = arg.replace(argType, "")
                while argName.startswith(" "):
                    argName = argName.removeprefix(" ")
                while argName.endswith(" "):
                    argName = argName.removesuffix(" ")
                argNames.append(argName)

        result.append(SIMDFunc(name, returnType, tuple(argTypes), tuple(argNames)))
    return result


def getCPrototype(function: SIMDFunc) -> str:
    result = ""

    result += f"{function.name}("
    for i in range(len(function.argTypes)):
        assert len(function.argTypes) == len(function.argNames)
        if i == len(function.argTypes) - 1:
            result += f"{function.argNames[i]}: {function.argTypes[i]}"
        else:
            result += f"{function.argNames[i]}: {function.argTypes[i]}, "
    result += f") -> {function.returnType}"

    return result


def myprint(functions: list[SIMDFunc]) -> None:
    print("[")
    for function in functions:
        print("    " + getCPrototype(function) + "\n")
    print("]")


def getArgParseData(argType: str, argId: int) -> tuple[str, bool]:
    """Code, Constant-require"""
    convert: str
    constantRequire = False

    match argType.removeprefix("const ").replace("void const", "void"):
        case "char":
            convert = f"PyFast_AsChar(*(((PyObject **) args) + {argId}))"
        case "short":
            convert = f"PyFast_AsShort(*(((PyObject **) args) + {argId}))"
        case "int":
            convert = f"PyFast_AsInt(*(((PyObject **) args) + {argId}))"
            constantRequire = True
        case "unsigned int":
            convert = f"(unsigned int) PyLong_AsUnsignedLong(*(((PyObject **) args) + {argId}))"
        case "unsigned":
            convert = f"(unsigned int) PyLong_AsUnsignedLong(*(((PyObject **) args) + {argId}))"
        case "float":
            convert = f"(float) PyFloat_AS_DOUBLE(*(((PyObject **) args) + {argId}))"
        case "double":
            convert = f"PyFloat_AS_DOUBLE(*(((PyObject **) args) + {argId}))"
        case "long":
            convert = f"PyLong_AsLong(*(((PyObject **) args) + {argId}))"
        case "long long":
            convert = f"PyLong_AsLongLong(*(((PyObject **) args) + {argId}))"
        case "unsigned long long":
            convert = f"PyLong_AsUnsignedLongLong(*(((PyObject **) args) + {argId}))"
        case "_MM_PERM_ENUM":
            # We will define this enum in python
            convert = f"PyFast_AsChar(*(((PyObject **) args) + {argId}))"
            constantRequire = True
        case "_MM_MANTISSA_NORM_ENUM":
            # We will define this enum in python
            return f"""
    _MM_MANTISSA_NORM_ENUM arg{argId};
    switch(PyFast_AsChar(*(((PyObject **) args) + {argId}))) {{
        case 0:
            arg{argId} = _MM_MANT_NORM_1_2;
            break;
        case 1:
            arg{argId} = _MM_MANT_NORM_p5_2;
            break;
        case 2:
            arg{argId} = _MM_MANT_NORM_p5_1;
            break;
        case 3:
            arg{argId} = _MM_MANT_NORM_p75_1p5;
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "Invalid value for arg {argId}");
            return nullptr;
    }}
""", True
        case "_MM_MANTISSA_SIGN_ENUM":
            # We will define this enum in python
            return f"""
    _MM_MANTISSA_SIGN_ENUM arg{argId};
    switch(PyFast_AsChar(*(((PyObject **) args) + {argId}))) {{
        case 0:
            arg{argId} = _MM_MANT_SIGN_src;
            break;
        case 1:
            arg{argId} = _MM_MANT_SIGN_zero;
            break;
        case 2:
            arg{argId} = _MM_MANT_SIGN_nan;
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "Invalid value for arg {argId}");
            return nullptr;
    }}
""", True
        case _:
            if argType.endswith("*"):
                convert = f"*(({argType}*) PyLong_AsVoidPtr(*(((PyObject **) args) + {argId})))"
            elif argType.startswith("__"):
                convert = f"*(({argType}*) PyLong_AsVoidPtr(*(((PyObject **) args) + {argId})))"
            else:
                raise NotImplementedError(argType)

    return f"    {argType} arg{argId} = ({argType}) {convert};", constantRequire


def getPyiType(cType: str) -> str:
    match cType.removeprefix("const ").replace("void const", "void"):
        case "char":
            return "int"
        case "short":
            return "int"
        case "int":
            return "int"
        case "unsigned int":
            return "int"
        case "unsigned":
            return "int"
        case "float":
            return "int"
        case "double":
            return "int"
        case "long":
            return "int"
        case "long long":
            return "int"
        case "unsigned long long":
            return "int"
        case "_MM_PERM_ENUM":
            return "MM_PERM_ENUM"
        case "_MM_MANTISSA_NORM_ENUM":
            return "MM_MANTISSA_NORM_ENUM"
        case "_MM_MANTISSA_SIGN_ENUM":
            return "MM_MANTISSA_SIGN_ENUM"
        case _:
            if cType.endswith("*"):
                return "Ptr"
            elif cType.startswith("__"):
                return "Ptr"
            else:
                raise NotImplementedError(cType)


def getCallCode(func: SIMDFunc, constantRequire: list[tuple[int, int, int]]) -> str:
    """func, constant require args (argId, immediateMin, immediateMax)"""
    args = ""
    argId = 0
    if func.returnType != "void":
        argId += 1

    for i in range(len(func.argTypes)):
        if i == len(func.argTypes) - 1:
            args += f"arg{argId}"
        else:
            args += f"arg{argId}, "
        argId += 1

    code: str
    if func.returnType != "void":
        code = f"    *(({func.returnType} *) PyLong_AsVoidPtr(*args)) = {func.name}({args});"
    else:
        code = f"    {func.name}({args});"

    if len(constantRequire) > 0:
        if len(constantRequire) > 4:
            raise NotImplementedError(len(constantRequire))

        for i, (argId, immediateMin, immediateMax) in enumerate(constantRequire):
            if immediateMin == immediateMax:
                code = code.replace(f"arg{argId}", str(immediateMin))
                continue

            result = f"switch (arg{argId}) {{\n"

            for value in range(immediateMin, immediateMax + 1):
                result += f"case {value}:\n"
                nextConstantRequire = constantRequire
                nextConstantRequire[i] = (argId, value, value)
                result += getCallCode(func, nextConstantRequire) + "\n"
                result += "break;\n"
            result += "default:\n"
            result += "PyErr_SetString(PyExc_ValueError, \"Invalid argument (out of range)\");\n"
            result += "return nullptr;\n"

            result += "}"

            return result
    return code


def main():
    with open("resources/avx512fintrin.h", "r") as f:
        codes = f.read()

    functions = getFunctions(codes)
    print(f"Found {len(functions)} methods.\n")
    functionsGenerated: int = 0
    immediateGenerated: int = 0
    functionsSkipped: int = 0

    function_def = ""
    c_function_def = ""
    function_register = ""

    for function in functions:
        funcCode = FUNC_TEMPLATE

        num_args = len(function.argTypes)
        argId = 0
        realArgId = 0

        if function.returnType != "void":
            num_args += 1
            argId += 1

        # arg parsing
        argParseCode = ""
        constantRequire: list[tuple[int, int, int]] = ObjectArrayList()
        curImmediateGenerated = 1
        for argType in function.argTypes:
            curCode, curRequire = getArgParseData(argType, argId)
            if ((curRequire and "_set_" not in function.name)
                    or (function.name in FORCE_CONSTANT and realArgId in FORCE_CONSTANT[function.name])):
                if function.name in IMMEDIATE_MAP:
                    data = next(data for data in IMMEDIATE_MAP[function.name] if data[0] == realArgId)
                    data = (data[0] + argId - realArgId, data[1], data[2])
                    curImmediateGenerated *= data[2] - data[1] + 1
                    constantRequire.append(data)
                else:
                    funcCode = FUNC_NOT_IMPL_TEMPLATE
                    functionsSkipped += 1
                    function.unsupported = True
            argParseCode += curCode + "\n"
            argId += 1
            realArgId += 1

        if curImmediateGenerated != 1:
            immediateGenerated += curImmediateGenerated

        funcCode = funcCode.replace("{num_args}", str(num_args))
        funcCode = funcCode.replace("{function_name}", function.name)
        funcCode = funcCode.replace("{arg_parsing}", argParseCode)

        # operation
        funcCode = funcCode.replace("{operation}", getCallCode(function, constantRequire))
        functionsGenerated += 1
        function_def += funcCode

        # c function
        cFuncCode = C_FUNC_TEMPLATE
        cFuncCode = cFuncCode.replace("{function_name}", function.name)
        c_function_def += cFuncCode

        # register
        regCode = REGISTER_TEMPLATE
        regCode = regCode.replace("{function_name}", function.name)
        function_register += regCode

    code = FILE_TEMPLATE
    code = code.replace("{function_def}", function_def)
    code = code.replace("{c_function_def}", c_function_def)
    code = code.replace("{function_register}", function_register)

    print(f"Generated {functionsGenerated} functions.")
    print(f"Generated {immediateGenerated} immediate branches.")
    print(f"Skipped {functionsSkipped} functions.\n")

    with open(RESULT_FILE, "w") as f:
        f.write(code)
    size = os.stat(RESULT_FILE).st_size
    lines = code.count("\n")
    print(f"Generated '{RESULT_FILE}' with {size} bytes and {lines} lines.")

    pyi = PYI_TEMPLATE

    pyiCodes = ""
    for function in functions:
        curPyi: str
        if function.unsupported:
            curPyi = PYI_FUNC_NOT_IMPL_TEMPLATE
        else:
            curPyi = PYI_FUNC_TEMPLATE

        if function.returnType != "void":
            args = "__result: Ptr, "
        else:
            args = ""
        for i in range(len(function.argTypes)):
            assert len(function.argTypes) == len(function.argNames)
            args += f"{function.argNames[i]}: {getPyiType(function.argTypes[i])}, "
        args = args.removesuffix(", ")

        curPyi = curPyi.replace("{function_name}", function.name)
        curPyi = curPyi.replace("{function_args}", args)
        curPyi = curPyi.replace("{prototype}", getCPrototype(function))

        pyiCodes += curPyi

    pyi = pyi.replace("{function_def}", pyiCodes)

    with open(RESULT_PYI_FILE, "w") as f:
        f.write(pyi)
    size = os.stat(RESULT_PYI_FILE).st_size
    lines = pyi.count("\n")
    print(f"Generated '{RESULT_PYI_FILE}' with {size} bytes and {lines} lines.")


if __name__ == '__main__':
    main()
