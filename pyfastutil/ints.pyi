from typing import overload, Iterable, SupportsIndex, Iterator


class IntArrayList(list[int]):
    """
    A specialized version of Python's list for integers, optimized for performance by using a C implementation.

    This class behaves similarly to the standard Python `list`, but it is specifically optimized for storing integers.
    It may offer better performance characteristics, especially for large data sets, due to its underlying C-based
    optimizations.

    The `IntArrayList` is restricted to storing integers (`int`), and it is designed to efficiently store values that
    fit within the range of standard C int types, which is typically `INT_MIN` to `INT_MAX`. Storing values outside
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
        - The `IntArrayList` is designed to store integers efficiently within the range of standard C int types
          (`INT_MIN` to `INT_MAX`). Storing values outside of this range is undefined.
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

    def to_list(self) -> list[int]:
        """
        Converts the `IntArrayList` to a standard Python list.

        This method returns a new list containing the same elements as the `IntArrayList` in the same order. The returned list
        will be a standard Python list, which can be used with all the usual Python list operations.

        Returns:
            list[int]: A new list containing all the elements of the `IntArrayList`.

        Example:
            >>> my_list = IntArrayList([1, 2, 3])
            >>> py_list = my_list.to_list()
            >>> print(py_list)
            [1, 2, 3]
        """
        pass


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


class BigIntArrayList(list[int]):
    """
    A specialized version of Python's list for integers, optimized for performance by using a C implementation.

    This class behaves similarly to the standard Python `list`, but it is specifically optimized for storing integers.
    It may offer better performance characteristics, especially for large data sets, due to its underlying C-based
    optimizations.

    The `BigIntArrayList` is restricted to storing integers (`long long`), and it is designed to efficiently store values that
    fit within the range of standard C long long types, which is typically `LONG_LONG_MIN` to `LONG_LONG_MAX`. Storing values outside
    of this range may result in undefined behavior.

    Parameters:
        - `exceptSize` (optional): The expected size of the list. This is used for preallocating memory to avoid
          frequent resizing when adding elements.
        - `iterable` (optional): An iterable of integers to initialize the list with.

    Example:
        >>> my_list = BigIntArrayList([1, 2, 3])
        >>> my_list.append(4)
        >>> print(my_list)
        [1, 2, 3, 4]

        >>> my_list = BigIntArrayList(10)  # Preallocate space for 10 elements
        >>> my_list.append(42)
        >>> print(my_list)
        [42]

    Note:
        - The `BigIntArrayList` is designed to store integers efficiently within the range of standard C long long types
          (`LONG_LONG_MIN` to `LONG_LONG_MAX`). Storing values outside of this range is undefined.
    """

    @overload
    def __init__(self, exceptSize: int) -> None:
        """
        Initializes an empty `BigIntArrayList` with a preallocated size.

        Parameters:
            exceptSize (int): The expected size of the list. This preallocates memory for the list to avoid frequent resizing
                              as elements are added.
        """
        pass

    @overload
    def __init__(self, iterable: Iterable[int], exceptSize: int) -> None:
        """
        Initializes an `BigIntArrayList` from an iterable of integers with a preallocated size.

        Parameters:
            iterable (Iterable[int]): An iterable of integers to initialize the list with.
            exceptSize (int): The expected size of the list. This preallocates memory for the list to avoid frequent resizing.
        """
        pass

    @overload
    def __init__(self) -> None:
        """
        Initializes an empty `BigIntArrayList` with no preallocated size.
        """
        pass

    @overload
    def __init__(self, iterable: Iterable[int]) -> None:
        """
        Initializes an `BigIntArrayList` from an iterable of integers.

        Parameters:
            iterable (Iterable[int]): An iterable of integers to initialize the list with.
        """
        pass

    @staticmethod
    @overload
    def from_range(__start: SupportsIndex, __stop: SupportsIndex, __step: SupportsIndex = 1) -> BigIntArrayList:
        """
        Creates an `BigIntArrayList` from a range of integers, similar to the built-in `range()` function.

        Parameters:
            __start (SupportsIndex): The starting value of the range (inclusive).
            __stop (SupportsIndex): The stopping value of the range (exclusive).
            __step (SupportsIndex, optional): The step between each value in the range. Defaults to 1.

        Returns:
            BigIntArrayList: A new `BigIntArrayList` object containing the integers in the specified range.

        Example:
            >>> BigIntArrayList.from_range(1, 10, 2)
            [1, 3, 5, 7, 9]
        """
        pass

    @staticmethod
    @overload
    def from_range(__stop: SupportsIndex) -> BigIntArrayList:
        """
        Creates an `BigIntArrayList` from a range of integers starting from 0 to the specified stop value.

        Parameters:
            __stop (SupportsIndex): The stopping value of the range (exclusive).

        Returns:
            BigIntArrayList: A new `BigIntArrayList` object containing the integers from 0 up to `__stop`.

        Example:
            >>> BigIntArrayList.from_range(5)
            [0, 1, 2, 3, 4]
        """
        pass

    def resize(self, __size: int) -> None:
        """
        Resizes the `BigIntArrayList` to the specified size.

        If the new size is larger than the current size, the list will be extended with zeros. If the new size is smaller,
        excess elements will be removed.

        Parameters:
            __size (int): The new size of the list.

        Example:
            >>> my_list = BigIntArrayList([1, 2, 3])
            >>> my_list.resize(5)
            >>> print(my_list)
            [1, 2, 3, 0, 0]

            >>> my_list.resize(2)
            >>> print(my_list)
            [1, 2]
        """
        pass

    def to_list(self) -> list[int]:
        """
        Converts the `BigIntArrayList` to a standard Python list.

        This method returns a new list containing the same elements as the `BigIntArrayList` in the same order. The returned list
        will be a standard Python list, which can be used with all the usual Python list operations.

        Returns:
            list[int]: A new list containing all the elements of the `BigIntArrayList`.

        Example:
            >>> my_list = BigIntArrayList([1, 2, 3])
            >>> py_list = my_list.to_list()
            >>> print(py_list)
            [1, 2, 3]
        """
        pass


class BigIntArrayListIter(Iterator[int]):
    """
    Iterator for BigIntArrayList.

    This class provides an iterator over an `BigIntArrayList`, allowing you to iterate over the elements
    of the list one by one.

    Note:
        This class cannot be directly instantiated by users. It is designed to be used internally by
        `BigIntArrayList` and can only be obtained by calling the `__iter__` method on an `BigIntArrayList` object.

    Example:
        >>> big_int_list = BigIntArrayList([1, 2, 3])
        >>> iter_obj = iter(big_int_list)  # This returns an BigIntArrayListIter instance
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

        This method retrieves the next element from the `BigIntArrayList` that this iterator is associated with.
        If all elements have been iterated over, it raises a `StopIteration` exception.

        Returns:
            int: The next integer in the `BigIntArrayList`.

        Raises:
            StopIteration: If there are no more elements to iterate over.
        """
        pass

class IntLinkedList(list[int]):
    """
    A specialized linked list for integers, optimized for efficient insertion and deletion.

    `IntLinkedList` is implemented as a doubly linked list, providing O(1) time complexity for insertions and deletions
    at both the head and tail, as well as for operations involving iterators. However, random access by index (e.g., `list[i]`)
    requires O(n) time, as the list must be traversed to reach the desired element.

    This class behaves similarly to Python's built-in `list`, but is specifically optimized for storing integers and for
    scenarios where frequent insertions and deletions are required, rather than random access.

    Example:
        >>> int_list = IntLinkedList()
        >>> int_list.append(1)
        >>> int_list.append(2)
        >>> int_list.insert(0, 3)
        >>> print(int_list)
        [3, 1, 2]

    Note:
        - Insertion and deletion are O(1).
        - Accessing elements by index is O(n).
        - Iteration over the list is O(1) per step, making it efficient for sequential access.

    Raises:
        TypeError: If an invalid operation is attempted.
    """

    @staticmethod
    @overload
    def from_range(__start: SupportsIndex, __stop: SupportsIndex, __step: SupportsIndex = 1) -> IntLinkedList:
        """
        Creates an `IntLinkedList` from a range of integers, similar to the built-in `range()` function.

        Parameters:
            __start (SupportsIndex): The starting value of the range (inclusive).
            __stop (SupportsIndex): The stopping value of the range (exclusive).
            __step (SupportsIndex, optional): The step between each value in the range. Defaults to 1.

        Returns:
            IntLinkedList: A new `IntLinkedList` object containing the integers in the specified range.

        Example:
            >>> IntLinkedList.from_range(1, 10, 2)
            [1, 3, 5, 7, 9]
        """
        pass

    @staticmethod
    @overload
    def from_range(__stop: SupportsIndex) -> IntLinkedList:
        """
        Creates an `IntLinkedList` from a range of integers starting from 0 to the specified stop value.

        Parameters:
            __stop (SupportsIndex): The stopping value of the range (exclusive).

        Returns:
            IntLinkedList: A new `IntLinkedList` object containing the integers from 0 up to `__stop`.

        Example:
            >>> IntLinkedList.from_range(5)
            [0, 1, 2, 3, 4]
        """
        pass

    def to_list(self) -> list[int]:
        """
        Converts the `IntLinkedList` to a standard Python list.

        This method returns a new list containing the same elements as the `IntLinkedList` in the same order. The returned list
        will be a standard Python list, which can be used with all the usual Python list operations.

        Returns:
            list[int]: A new list containing all the elements of the `IntLinkedList`.

        Example:
            >>> int_list = IntLinkedList([1, 2, 3])
            >>> py_list = int_list.to_list()
            >>> print(py_list)
            [1, 2, 3]
        """
        pass

class IntLinkedListIter(Iterator[int]):
    """
    Iterator for `IntLinkedList`.

    This class provides an iterator over an `IntLinkedList`, allowing you to iterate over the elements
    of the list one by one in O(1) time per step. The iterator maintains a reference to the list and iterates
    through it in constant time for each element, making it very efficient for sequential access.

    Note:
        This class cannot be directly instantiated by users. It is designed to be used internally by
        `IntLinkedList` and can only be obtained by calling the `__iter__` method on an `IntLinkedList` object.

    Example:
        >>> int_list = IntLinkedList([1, 2, 3])
        >>> iter_obj = iter(int_list)  # This returns an IntLinkedListIter instance
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

        This method retrieves the next element from the `IntLinkedList` that this iterator is associated with.
        If all elements have been iterated over, it raises a `StopIteration` exception.

        Returns:
            int: The next integer in the `IntLinkedList`.

        Raises:
            StopIteration: If there are no more elements to iterate over.
        """
        pass

class IntIntHashMap(dict[int, int]):
    pass
