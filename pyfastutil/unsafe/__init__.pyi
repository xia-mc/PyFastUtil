from typing import TypeVar
from .Unsafe import Unsafe
from .SIMD import SIMD
from .SIMDLowAVX512 import SIMDLowAVX512
from .ASM import ASM

from .__SIMDLowAVX512Defines import *

Ptr = TypeVar("Ptr", bound=int)
NULL: Ptr

Unsafe: type[Unsafe]
SIMD: type[SIMD]
SIMDLowAVX512: type[SIMDLowAVX512]
ASM: type[ASM]
