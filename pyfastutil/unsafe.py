from typing import TypeVar

# noinspection PyUnresolvedReferences
from .__pyfastutil import Unsafe as __Unsafe
# noinspection PyUnresolvedReferences
from .__pyfastutil import SIMD as __SIMD

Ptr = TypeVar("Ptr", bound=int)
NULL = 0

Unsafe = __Unsafe.Unsafe
SIMD = __SIMD.SIMD
