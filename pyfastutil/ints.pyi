from typing import overload, Iterable, SupportsIndex, final


@final
class IntArrayList(list[int]):
    """
    A specialized version of Python's list for integers, optimized for performance by using a C implementation.

    This class behaves similarly to the standard list, but is specifically optimized for integer storage,
    offering potentially better performance characteristics, especially for large data sets.

    Note:
        - The IntList is designed to store integers efficiently within the range of standard C long types,
        which is typically LONG_MIN to LONG_MAX. Storing values outside of this range is undefined.

    Example:
        >>> my_list = IntArrayList([1, 2, 3])
        >>> my_list.append(4)
        >>> print(my_list)
        [1, 2, 3, 4]
    """

    @overload
    def __init__(self, size: int): ...

    @overload
    def __init__(self, __iterable: Iterable[int], size: int) -> None: ...

    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, __iterable: Iterable[int]) -> None: ...

    @overload
    def from_range(self, __start: SupportsIndex, __stop: SupportsIndex, __step: SupportsIndex = 1): ...

    @overload
    def from_range(self, __stop: SupportsIndex): ...
