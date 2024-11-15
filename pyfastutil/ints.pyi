from typing import overload, Iterable, SupportsIndex, final, Iterator, _T_co


@final
class IntArrayList(list[int]):
    """
    A specialized version of Python's list for integers, optimized for performance by using a C implementation.

    This class behaves similarly to the standard Python `list`, but it is specifically optimized for storing integers.
    It may offer better performance characteristics, especially for large data sets, due to its underlying C-based
    optimizations.

    The `IntArrayList` is restricted to storing integers (`int`), and it is designed to efficiently store values that
    fit within the range of standard C long types, which is typically `LONG_MIN` to `LONG_MAX`. Storing values outside
    of this range may result in undefined behavior.

    Parameters:
        - `exceptSize` (optional): The expected size of the list. This is used for preallocating memory to avoid
          frequent resizing when adding elements.
        - `iterable` (optional): An iterable of integers to initialize the list with.

    Example:
        >>> my_list = IntArrayList([1, 2, 3])
        >>> my_list.append(4)
        >>> print(my_list)
        [1, 2, 3, 4]

        >>> my_list = IntArrayList(10)  # Preallocate space for 10 elements
        >>> my_list.append(42)
        >>> print(my_list)
        [42]

    Note:
        - The `IntArrayList` is designed to store integers efficiently within the range of standard C long types
          (`LONG_MIN` to `LONG_MAX`). Storing values outside of this range is undefined.
    """

    @overload
    def __init__(self, exceptSize: int) -> None:
        """
        Initializes an empty `IntArrayList` with a preallocated size.

        Parameters:
            exceptSize (int): The expected size of the list. This preallocates memory for the list to avoid frequent resizing
                              as elements are added.
        """
        pass

    @overload
    def __init__(self, iterable: Iterable[int], exceptSize: int) -> None:
        """
        Initializes an `IntArrayList` from an iterable of integers with a preallocated size.

        Parameters:
            iterable (Iterable[int]): An iterable of integers to initialize the list with.
            exceptSize (int): The expected size of the list. This preallocates memory for the list to avoid frequent resizing.
        """
        pass

    @overload
    def __init__(self) -> None:
        """
        Initializes an empty `IntArrayList` with no preallocated size.
        """
        pass

    @overload
    def __init__(self, iterable: Iterable[int]) -> None:
        """
        Initializes an `IntArrayList` from an iterable of integers.

        Parameters:
            iterable (Iterable[int]): An iterable of integers to initialize the list with.
        """
        pass

    @staticmethod
    @overload
    def from_range(__start: SupportsIndex, __stop: SupportsIndex, __step: SupportsIndex = 1) -> IntArrayList:
        """
        Creates an `IntArrayList` from a range of integers, similar to the built-in `range()` function.

        Parameters:
            __start (SupportsIndex): The starting value of the range (inclusive).
            __stop (SupportsIndex): The stopping value of the range (exclusive).
            __step (SupportsIndex, optional): The step between each value in the range. Defaults to 1.

        Returns:
            IntArrayList: A new `IntArrayList` object containing the integers in the specified range.

        Example:
            >>> IntArrayList.from_range(1, 10, 2)
            [1, 3, 5, 7, 9]
        """
        pass

    @staticmethod
    @overload
    def from_range(__stop: SupportsIndex) -> IntArrayList:
        """
        Creates an `IntArrayList` from a range of integers starting from 0 to the specified stop value.

        Parameters:
            __stop (SupportsIndex): The stopping value of the range (exclusive).

        Returns:
            IntArrayList: A new `IntArrayList` object containing the integers from 0 up to `__stop`.

        Example:
            >>> IntArrayList.from_range(5)
            [0, 1, 2, 3, 4]
        """
        pass

    def resize(self, __size: int) -> None:
        """
        Resizes the `IntArrayList` to the specified size.

        If the new size is larger than the current size, the list will be extended with zeros. If the new size is smaller,
        excess elements will be removed.

        Parameters:
            __size (int): The new size of the list.

        Example:
            >>> my_list = IntArrayList([1, 2, 3])
            >>> my_list.resize(5)
            >>> print(my_list)
            [1, 2, 3, 0, 0]

            >>> my_list.resize(2)
            >>> print(my_list)
            [1, 2]
        """
        pass


@final
class IntArrayListIter(Iterator[int]):
    """
    Iterator for IntArrayList.

    This class provides an iterator over an `IntArrayList`, allowing you to iterate over the elements
    of the list one by one.

    Note:
        This class cannot be directly instantiated by users. It is designed to be used internally by
        `IntArrayList` and can only be obtained by calling the `__iter__` method on an `IntArrayList` object.

    Example:
        >>> int_list = IntArrayList([1, 2, 3])
        >>> iter_obj = iter(int_list)  # This returns an IntArrayListIter instance
        >>> next(iter_obj)
        1
        >>> next(iter_obj)
        2
        >>> next(iter_obj)
        3
        >>> next(iter_obj)  # Raises StopIteration

    Raises:
        TypeError: If attempted to be instantiated directly.
    """

    def __next__(self) -> int:
        """
        Return the next element in the iteration.

        This method retrieves the next element from the `IntArrayList` that this iterator is associated with.
        If all elements have been iterated over, it raises a `StopIteration` exception.

        Returns:
            int: The next integer in the `IntArrayList`.

        Raises:
            StopIteration: If there are no more elements to iterate over.
        """
        pass
