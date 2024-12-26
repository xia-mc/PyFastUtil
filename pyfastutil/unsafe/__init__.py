from typing import TypeVar

# noinspection PyUnresolvedReferences
from ..__pyfastutil import Unsafe as __Unsafe
# noinspection PyUnresolvedReferences
from ..__pyfastutil import ASM as __ASM
# noinspection PyUnresolvedReferences
from ..__pyfastutil import SIMD as __SIMD
# noinspection PyUnresolvedReferences
from ..__pyfastutil import SIMDLowAVX512 as __SIMDLowAVX512

from .__SIMDLowAVX512Defines import *

Ptr = TypeVar("Ptr", bound=int)
NULL = 0

Unsafe = __Unsafe.Unsafe
ASM = __ASM.ASM
SIMD = __SIMD.SIMD
SIMDLowAVX512 = __SIMDLowAVX512.SIMDLowAVX512