from typing import TypeVar

# noinspection PyUnresolvedReferences
from .__pyfastutil import Unsafe as __Unsafe

Ptr = TypeVar("Ptr", bound=int)
NullPtr: Ptr = 0

Unsafe = __Unsafe.Unsafe
