from typing import overload, Iterable, SupportsIndex, final, Iterator


@final
class ObjectArrayList(list):
    """
    A specialized version of Python's `list` that can store any type of object, optimized for performance by using a C implementation.

    This class behaves similarly to the standard Python `list`, but it is designed for use cases where performance is critical.
    It can store objects of any type, including integers, strings, custom objects, and more, making it as flexible as Python's
    built-in `list` type.

    The `ObjectArrayList` is optimized for performance, especially for large datasets, by utilizing underlying C-based optimizations.
    It supports all standard list operations such as indexing, slicing, appending, and iteration.

    Parameters:
        - `exceptSize` (optional): The expected size of the list. This is used for preallocating memory to avoid frequent resizing
          when adding elements.
        - `iterable` (optional): An iterable of objects to initialize the list with.

    Example:
        >>> obj_list = ObjectArrayList([1, "hello", 3.14])
        >>> obj_list.append({"key": "value"})
        >>> print(obj_list)
        [1, "hello", 3.14, {"key": "value"}]

        >>> obj_list = ObjectArrayList(10)  # Preallocate space for 10 elements
        >>> obj_list.append(42)
        >>> print(obj_list)
        [42]

    Note:
        - Unlike `IntArrayList`, `ObjectArrayList` can store objects of any type, making it a more flexible data structure.
    """

    @overload
    def __init__(self, exceptSize: int) -> None:
        """
        Initializes an empty `ObjectArrayList` with a preallocated size.

        Parameters:
            exceptSize (int): The expected size of the list. This preallocates memory for the list to avoid frequent resizing
                              as elements are added.
        """
        pass

    @overload
    def __init__(self, iterable: Iterable[object], exceptSize: int) -> None:
        """
        Initializes an `ObjectArrayList` from an iterable of objects with a preallocated size.

        Parameters:
            iterable (Iterable[object]): An iterable of objects to initialize the list with.
            exceptSize (int): The expected size of the list. This preallocates memory for the list to avoid frequent resizing.
        """
        pass

    @overload
    def __init__(self) -> None:
        """
        Initializes an empty `ObjectArrayList` with no preallocated size.
        """
        pass

    @overload
    def __init__(self, iterable: Iterable[object]) -> None:
        """
        Initializes an `ObjectArrayList` from an iterable of objects.

        Parameters:
            iterable (Iterable[object]): An iterable of objects to initialize the list with.
        """
        pass

    def resize(self, __size: int) -> None:
        """
        Resizes the `ObjectArrayList` to the specified size.

        If the new size is larger than the current size, the list will be extended with `None` values. If the new size is
        smaller, excess elements will be removed.

        Parameters:
            __size (int): The new size of the list.

        Example:
            >>> obj_list = ObjectArrayList([1, "hello", 3.14])
            >>> obj_list.resize(5)
            >>> print(obj_list)
            [1, "hello", 3.14, None, None]

            >>> obj_list.resize(2)
            >>> print(obj_list)
            [1, "hello"]
        """
        pass


@final
class ObjectArrayListIter(Iterator):
    """
    Iterator for `ObjectArrayList`.

    This class provides an iterator over an `ObjectArrayList`, allowing you to iterate over the elements
    of the list one by one.

    Note:
        This class cannot be directly instantiated by users. It is designed to be used internally by
        `ObjectArrayList` and can only be obtained by calling the `__iter__` method on an `ObjectArrayList` object.

    Example:
        >>> obj_list = ObjectArrayList([1, "hello", 3.14])
        >>> iter_obj = iter(obj_list)  # This returns an ObjectArrayListIter instance
        >>> next(iter_obj)
        1
        >>> next(iter_obj)
        "hello"
        >>> next(iter_obj)
        3.14
        >>> next(iter_obj)  # Raises StopIteration

    Raises:
        TypeError: If attempted to be instantiated directly.
    """

    def __next__(self) -> object:
        """
        Return the next element in the iteration.

        This method retrieves the next element from the `ObjectArrayList` that this iterator is associated with.
        If all elements have been iterated over, it raises a `StopIteration` exception.

        Returns:
            object: The next object in the `ObjectArrayList`.

        Raises:
            StopIteration: If there are no more elements to iterate over.
        """
        pass
