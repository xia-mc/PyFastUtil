from types import FunctionType
from typing import TypeVar, overload, Callable

from ._internal import native as _native

__T = TypeVar("__T", bound=FunctionType)


@overload
def native(func: __T) -> __T: ...


@overload
def native(c_int: bool = False) -> Callable[[__T], __T]: ...


def native(arg: __T | bool = None):
    return _native(arg) if type(arg) is FunctionType else _native(arg, c_int=True)
