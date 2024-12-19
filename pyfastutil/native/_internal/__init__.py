import hashlib
import importlib.util
import inspect
import os
import typing
from dis import Bytecode
from pathlib import Path
from types import FunctionType, BuiltinFunctionType, ModuleType
from typing import Iterable
from inspect import Parameter

from . import Compiler, BytecodeTranslator


NATIVE_CACHE: dict[str, BuiltinFunctionType | FunctionType] = {}


def native(func: FunctionType) -> BuiltinFunctionType | FunctionType:
    # initialization
    params = inspect.signature(func).parameters.values()
    bytecode = Bytecode(func)
    hashed = _hashFunc(params, bytecode)
    if hashed in NATIVE_CACHE:
        return NATIVE_CACHE[hashed]

    moduleName = f"native_{hashed}"
    cacheFolder = Path("__nativecache__")
    os.makedirs(cacheFolder, exist_ok=True)

    # check if already compiled
    resultPath = Path(cacheFolder, moduleName + ".pyd")
    if resultPath.exists():
        res = getattr(_importModuleFromPath(moduleName, str(resultPath.absolute())), func.__name__)
        NATIVE_CACHE[hashed] = res
        return typing.cast(BuiltinFunctionType, res)

    # compile
    try:
        Compiler.compileCode(cacheFolder, func.__name__, moduleName, BytecodeTranslator.toC(func, params, bytecode))
    except NotImplementedError:
        NATIVE_CACHE[hashed] = func
        return func

    resultPath = Path(cacheFolder, moduleName + ".pyd")
    if resultPath.exists():
        res = getattr(_importModuleFromPath(moduleName, str(resultPath.absolute())), func.__name__)
        NATIVE_CACHE[hashed] = res
        return typing.cast(BuiltinFunctionType, res)
    return func


def _hashFunc(arguments: Iterable[Parameter], bytecode: Bytecode) -> str:
    return hashlib.sha256(
        (
            str([str(obj) for obj in arguments])  # params
            + bytecode.dis()
        ).encode(errors="ignore")).hexdigest()


def _importModuleFromPath(moduleName: str, filePath: str) -> ModuleType:
    spec = importlib.util.spec_from_file_location(moduleName, filePath)
    if spec is None:
        raise ImportError(f"Cannot find module {moduleName} at {filePath}")

    module = importlib.util.module_from_spec(spec)

    spec.loader.exec_module(module)

    return module
