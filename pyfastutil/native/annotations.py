from types import FunctionType
from typing import TypeVar

from ._internal import native as _native

__T = TypeVar("__T", bound=FunctionType)


def native(func: __T) -> __T:
    """
    A decorator that attempts to optimize a Python function by
    translating it into a native implementation.

    **Note**: This decorator is an experimental prototype and has been
    abandoned. It is incomplete, lacks proper maintenance, and may not
    work as expected in all cases. Use it at your own risk.

    Parameters:
        func (__T): The Python function to be optimized.

    Returns:
        __T: The (potentially) optimized function.

    Warning:
        This decorator has been deprecated and is no longer maintained.
        It is provided for demonstration purposes only and should not
        be used in production code.

    Example:
        @native
        def add(a, b):
            return a + b
    """
    return _native(func)
