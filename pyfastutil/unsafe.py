from typing import TypeVar

# noinspection PyUnresolvedReferences
from .__pyfastutil import Unsafe as __Unsafe

Ptr = TypeVar("Ptr", bound=int)

Unsafe = __Unsafe.Unsafe
