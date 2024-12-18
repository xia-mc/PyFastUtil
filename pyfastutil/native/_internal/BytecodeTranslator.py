import builtins
import ctypes
import sys
import types
from dis import Bytecode
from inspect import Parameter
from types import FunctionType
from typing import Callable, Any, Optional, Iterable

from pyfastutil.objects import ObjectArrayList

INT_MAX = (2 ** (ctypes.sizeof(ctypes.c_int) * 8 - 1)) - 1
INT_MIN = -INT_MAX - 1


def intToDigits(value: int) -> tuple[int, int, list[int]]:
    """
    将一个 Python 的 int 分解为 _PyLong_FromDigits 所需的 digit 信息。
    :returns:
    - sign: 0 表示正数，1 表示负数
    - num_digits: digit 数组的长度
    - digits: 包含每个 digit 的数组
    """
    DIGIT_BITS = 15 if sys.maxsize > 2 ** 32 else 30
    DIGIT_BASE = 1 << DIGIT_BITS

    sign = 0 if value >= 0 else 1
    value = abs(value)

    digits = []
    while value:
        digits.append(value % DIGIT_BASE)
        value //= DIGIT_BASE

    if not digits:
        digits = [0]

    return sign, len(digits), digits


class _BytecodeTranslator:
    def __init__(self, func: FunctionType, arguments: Iterable[Parameter], bytecode: Bytecode):
        self.bytecode = bytecode
        self.vars: tuple[str, ...] = func.__code__.co_varnames
        self.code: list[str] = ObjectArrayList(100)
        self.delayActions: list[Callable[[], None]] = []

        self.constsCount: int = 0
        self.constants: dict[object, str] = {}

        self.regCount: int = func.__code__.co_stacksize
        self.regUsed: int = 0
        self.regAutoclose: list[bool] = [False] * self.regCount

        # helper context
        self.kwNames: list[str] = []  # to support kwargs call
        self.forDepth: int = 0  # to support for
        self.bytecodeOffsets: dict[int, int] = {}  # to support JUMP_BACKWARD
        """key: bytecode bytes offset (f->lastI), value: c code offset (line)"""
        self.varsUnused: set[str] = set(self.vars)

        # define local variables (include args)
        for name in self.varsUnused:
            self.append(f"PyObject *var_{name} = nullptr;")

        # define registers
        for i in range(self.regCount):
            self.append(f"[[maybe_unused]] PyObject *tmp{i + 1};")
        self.append("")

        # parse args
        for i, param in enumerate(arguments):
            if param.kind == Parameter.KEYWORD_ONLY:
                raise NotImplementedError(f"Param '{param.name}'")

            if param.default == Parameter.empty:
                self.assign(param.name, f"*(args + {i})")
            else:
                expr, shouldDecref = self.fromConstant(param.default)
                self.assign(param.name, f"nargs > {i + 1} ? *(args + {i}) : {expr}")

    def append(self, code: str) -> None:
        self.code.append(code)

    def handleDelayActions(self) -> None:
        for action in self.delayActions:
            action()
        self.delayActions.clear()

    def pushCall(self, expr: str, autoclose: bool) -> None:
        self.append(f"res = {expr};")
        self.delayActions.append(lambda: self.push("res", autoclose))

    def push(self, expr: str, autoclose: bool) -> None:
        assert 0 <= self.regUsed <= self.regCount - 1
        if autoclose:
            self.regAutoclose[self.regUsed] = True

        self.regUsed += 1
        self.append(f"tmp{self.regUsed} = {expr};")

    def pop(self, noAutoClose: bool = False) -> str:
        regName = self.back()

        self.regUsed -= 1
        if not noAutoClose and self.regAutoclose[self.regUsed]:
            self.delayActions.append(lambda: self.append(f"PyFast_DECREF({regName});"))
        return regName

    def back(self) -> str:
        assert 1 <= self.regUsed <= self.regCount
        return f"tmp{self.regUsed}"

    def assign(self, name: str, expr: str) -> None:
        if name not in self.varsUnused:
            self.append(f"PyFast_DECREF(var_{name});")
        else:
            self.varsUnused.remove(name)
        name = f"var_{name}"
        self.append(f"{name} = {expr};")
        self.append(f"PyFast_INCREF({name});")

    def name(self, name: str) -> Optional[str]:
        if name in self.varsUnused:
            return None
        return f"var_{name}"

    def assignConstant(self, expr: str, value: object = None) -> str:
        if value is not None and value in self.constants:
            return self.constants[value]

        self.constsCount += 1
        name = f"constant{self.constsCount}"
        self.append(f"static auto {name} = {expr};")

        if value is not None:
            self.constants[value] = name
        return name

    @staticmethod
    def call(name: str, *args: Any) -> str:
        formattedArgs = ", ".join(map(str, args))
        return f"{name}({formattedArgs})"

    def returnVal(self, expr: str) -> None:
        self.handleDelayActions()
        for name in self.vars:
            name = self.name(name)
            if name is None:
                continue
            self.append(f"PyFast_XDECREF({name});")
        self.append(f"return {expr};")

    def fromConstant(self, value: Any, forceConstant: bool = False) -> tuple[str, bool]:
        """expr code, should decref"""
        match type(value):
            case builtins.int:
                if INT_MIN <= value <= INT_MAX:
                    return self.assignConstant(self.call(f"PyFast_FromInt", value), value), False
                elif sys.getsizeof(value) <= 16 or forceConstant:
                    sign, num_digits, digits = intToDigits(value)
                    formattedDigits = "{" + ", ".join(map(str, digits)) + "}"
                    return self.assignConstant(
                        self.call(f"PyFast_FromDigits", value, sign, num_digits, formattedDigits), value
                    ), False
                else:
                    sign, num_digits, digits = intToDigits(value)
                    formattedDigits = "{" + ", ".join(map(str, digits)) + "}"
                    return self.call(f"PyFast_FromDigits", value, sign, num_digits, formattedDigits), True
            case builtins.bool:
                return "Py_True" if value else "Py_False", False
            case types.NoneType:
                return "Py_None", False
            case builtins.str:
                if len(value) <= 16 or forceConstant:
                    return self.assignConstant(self.call("PyUnicode_FromString", f"\"{value}\""), value), False
                return self.call("PyUnicode_FromString", f"\"{value}\""), True
            case _:
                raise NotImplementedError(value)

    def run(self) -> list[str]:
        for instr in self.bytecode:
            op = instr.opname
            arg = instr.arg
            argVal = instr.argval

            self.append("")
            self.append(f"// {op}({arg}, {argVal})")

            self.bytecodeOffsets[instr.offset] = len(self.code)

            self.visit(op, arg, argVal)

            self.handleDelayActions()

        return self.code

    def visit(self, op: str, arg: Optional[int], argVal: Any) -> None:
        match op:
            case "RESUME":
                # We needn't PyGIL_Ensure
                ...

            case "NOP":
                ...

            case "LOAD_FAST":
                self.push(f"var_{argVal}", True)
                self.append(f"PyFast_INCREF({self.back()});")

            case "STORE_FAST":
                self.assign(argVal, self.pop())

            case "LOAD_CONST":
                self.push(*self.fromConstant(argVal))

            case "LOAD_GLOBAL":
                name, _ = self.fromConstant(argVal, forceConstant=True)
                hashVal = self.assignConstant(self.call("PyObject_Hash", name))
                self.pushCall(self.call("PyFast_LoadGlobal", name, hashVal), True)

            case "BINARY_OP":
                right = self.pop()
                left = self.pop()

                match arg:
                    case 0:  # +
                        self.pushCall(self.call("PyNumber_Add", left, right), True)
                    case 5:  # *
                        self.pushCall(self.call("PyNumber_Multiply", left, right), True)
                    case 13:  # +=
                        self.pushCall(self.call("PyNumber_Add", left, right), True)
                    case _:
                        raise NotImplementedError(op, arg, argVal)

            case "RETURN_VALUE":
                while self.regUsed > 1:
                    self.pop()
                self.returnVal(self.pop(noAutoClose=True))

            case "RETURN_CONST":
                self.returnVal(self.fromConstant(argVal, forceConstant=True)[0])

            case "KW_NAMES":  # cpython 3.11+ only
                self.kwNames.append(argVal[0])

            case "CALL":
                args: list[str] = [self.pop() for _ in range(argVal)]
                args.reverse()

                if len(args) == 0:
                    self.pushCall(self.call("PyFast_CallNoArgs", self.pop()), True)
                elif len(self.kwNames) == 0:
                    self.pushCall(self.call("PyFast_CallNoKwargs", self.pop(), "{" + ", ".join(args) + "}"), True)
                else:
                    posArgsCount: int = len(args) - len(self.kwNames)

                    kwNamesLst = kwNamesHashLst = []
                    for name in self.kwNames:
                        nameObj, _ = self.fromConstant(name, forceConstant=True)
                        kwNamesLst.append(nameObj)
                        hashObj = self.assignConstant(self.call("PyObject_Hash", nameObj))
                        kwNamesHashLst.append(hashObj)

                    kwNames = self.assignConstant(self.call("PyFast_TuplePack", "{" + ", ".join(kwNamesLst) + "}"))
                    kwNamesHash = "{" + ", ".join(kwNamesHashLst) + "}"

                    self.pushCall(self.call(
                        "PyFast_Call",
                        self.pop(),
                        "{" + ", ".join(args) + "}",
                        posArgsCount, kwNames, kwNamesHash
                    ), True)

            case "POP_TOP":
                self.pop()

            case "LOAD_ATTR":
                self.pushCall(self.call(
                    "PyObject_GetAttr",
                    self.pop(),
                    self.fromConstant(argVal, forceConstant=True)[0]
                ), True)

            case "GET_ITER":
                self.pushCall(self.call("PyFast_GetIter", self.pop()), True)

            case "FOR_ITER":
                self.forDepth += 1
                self.pushCall(self.call("PyFast_Next", self.back()), True)
                self.append(f"if (UNLIKELY(res == nullptr)) {{")
                # We needn't to decref the result, because it's nullptr
                self.append(f"    PyErr_Clear();")
                self.append(f"    goto endFor{self.forDepth};")
                self.append(f"}}")

            case "END_FOR":
                self.append(f"endFor{self.forDepth}:")
                self.pop()  # pop iter
                self.forDepth -= 1

            case "JUMP_BACKWARD":
                codePos = self.bytecodeOffsets[argVal]
                label = f"jumpLastI{argVal}"
                if self.code[codePos] != label:
                    self.code.insert(codePos, f"{label}:")
                self.append(f"goto {label};")

            case _:
                raise NotImplementedError(op, arg, argVal)


# noinspection PyUnusedLocal
def toC(func: FunctionType, arguments: Iterable[Parameter], bytecode: Bytecode, c_int: bool) -> list[str]:
    return _BytecodeTranslator(func, arguments, bytecode).run()
