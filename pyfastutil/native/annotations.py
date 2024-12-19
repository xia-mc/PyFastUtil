from types import FunctionType
from typing import TypeVar

from ._internal import native as _native

__T = TypeVar("__T", bound=FunctionType)


def native(func: __T) -> __T:
    return _native(func)
