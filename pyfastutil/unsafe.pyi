from typing import final, TypeVar

Ptr = TypeVar("Ptr", bound=int)
NullPtr: Ptr

@final
class Unsafe:
    """
    A class for performing low-level, unsafe memory operations. This class allows direct memory
    management and manipulation, akin to C-style malloc, free, and memcpy. It should be used with
    extreme caution, as improper use can lead to memory corruption, crashes, or other undefined behavior.

    **Best Practice**:
        Use this class within a `with` statement to limit the scope of unsafe operations, similar to
        the concept of `unsafe` blocks in Rust. This ensures that unsafe actions are confined to a
        small, controlled section of code.

        Example:
        ```python
        with Unsafe() as unsafe:
            # Perform unsafe operations here
        ```

    **Warning**:
        This class bypasses Python's automatic memory management and should only be used by advanced
        users who understand the risks of manual memory management.
    """

    def __init__(self) -> None:
        """
        Initializes the Unsafe context. This constructor does not allocate any memory or perform any
        unsafe operations. It simply prepares the object to be used in a `with` statement.
        """
        pass

    def __enter__(self) -> Unsafe: ...

    def __exit__(self, exc_type, exc_val, exc_tb) -> None: ...

    def malloc(self, __size: int) -> Ptr:
        """
        Allocates a block of memory of the given size.

        :param __size: The number of bytes to allocate.
        :return: A pointer (integer representing the memory address) to the allocated memory block.

        **Warning**: The allocated memory is uninitialized. You are responsible for managing this memory
        and freeing it when no longer needed.
        """
        pass

    def calloc(self, __num: int, __size: int) -> Ptr:
        """
        Allocates a block of memory for an array of elements, initializing all bytes to zero.

        :param __num: The number of elements to allocate.
        :param __size: The size of each element in bytes.
        :return: A pointer (integer representing the memory address) to the allocated and initialized memory block.

        **Warning**: You are responsible for managing this memory and freeing it when no longer needed.
        """
        pass

    def realloc(self, __pointer: Ptr, __newSize: int) -> Ptr:
        """
        Reallocates a block of memory to a new size.

        :param __pointer: The pointer to the previously allocated memory block.
        :param __newSize: The new size of the memory block in bytes.
        :return: A pointer (integer representing the memory address) to the reallocated memory block.

        **Warning**: You are responsible for managing this memory and freeing it when no longer needed.
        """
        pass

    def free(self, __pointer: Ptr) -> None:
        """
        Frees a previously allocated block of memory.

        :param __pointer: The pointer to the memory block to free.

        **Warning**: After calling this function, the pointer becomes invalid and should not be used.
        """
        pass

    def get(self, __pointer: Ptr, __size: int) -> bytes:
        """
        Reads a block of memory and returns its contents as a bytes object.

        :param __pointer: The pointer to the memory block to read from.
        :param __size: The number of bytes to read.
        :return: A bytes object containing the data read from memory.

        **Warning**: Ensure that the memory block is valid and accessible. Accessing invalid memory
        can cause undefined behavior.
        """
        pass

    def set(self, __pointer: Ptr, __bytes: bytes) -> None:
        """
        Writes a bytes object to a block of memory.

        :param __pointer: The pointer to the memory block to write to.
        :param __bytes: The bytes object containing the data to write.

        **Warning**: Ensure that the memory block is large enough to hold the data. Writing beyond
        the allocated memory can cause undefined behavior.
        """
        pass

    def get_address(self, __object: object) -> Ptr:
        """
        Returns the memory address of a Python object.

        :param __object: The Python object to get the address of.
        :return: A pointer (integer representing the memory address) of the object.

        **Warning**: Directly manipulating the memory of Python objects can lead to interpreter crashes
        and undefined behavior. Use with caution.
        """
        pass

    def as_object(self, __pointer: Ptr) -> object:
        """
        Converts a memory address (pointer) back into a Python object.

        This method allows you to interpret the raw memory at the given pointer as a Python object. It is
        the inverse of `get_address()`, which retrieves the memory address of a Python object.

        :param __pointer: The pointer (memory address) to the object.
        :return: The Python object located at the given memory address.

        **Warning**: This is an extremely unsafe operation. Only use this method if you are absolutely
        certain that the memory address points to a valid Python object. Incorrect use can lead to
        interpreter crashes, memory corruption, or undefined behavior.
        """
        pass

    def memcpy(self, __from: Ptr, __to: Ptr, __size: int) -> None:
        """
        Copies a block of memory from one location to another.

        :param __from: The pointer to the source memory block.
        :param __to: The pointer to the destination memory block.
        :param __size: The number of bytes to copy.

        **Warning**: Ensure that both the source and destination memory blocks are valid and large enough
        to hold the data being copied. Copying invalid memory can cause undefined behavior.
        """
        pass

    def incref(self, __object: object) -> None:
        """
        Increments the reference count of a Python object.

        :param __object: The Python object whose reference count will be incremented.

        **Warning**: Misusing reference counts can lead to memory leaks or premature object destruction.
        """
        pass

    def decref(self, __object: object) -> None:
        """
        Decrements the reference count of a Python object.

        :param __object: The Python object whose reference count will be decremented.

        **Warning**: Misusing reference counts can lead to memory leaks or premature object destruction.
        """
        pass

    def refcnt(self, __object: object) -> int:
        """
        Get the reference count of a Python object.

        :param __object: The Python object.
        """
        pass

    def fputs(self, __str: str) -> None:
        """
        Writes a string to a low-level output stream.

        This method writes the given string to a file or output stream, similar to the C `fputs` function.
        The exact behavior depends on the underlying implementation of the output stream.

        :param __str: The string to write to the output stream.

        **Warning**: This method performs a low-level write operation, so it does not handle automatic
        newline conversion or character encoding. Ensure the string is in the correct format before writing.
        """
        pass

    def fflush(self) -> None:
        """
        Flushes the output buffer of the stream.

        This method forces any buffered output data to be written to the underlying device or file.
        It is similar to the C `fflush` function.

        **Use Case**: This is useful when you want to ensure that all data written to the stream is
        physically written out (for example, when writing to a file or sending data over a network).

        **Warning**: Improper use of this method can lead to performance degradation if called too frequently.
        """
        pass

    def fgets(self, __bufferSize: int) -> str:
        """
        Reads a line from a low-level input stream into a buffer.

        This method reads up to `__bufferSize - 1` characters from the input stream, stopping when a newline
        character is encountered or the buffer is full. It is similar to the C `fgets` function.

        :param __bufferSize: The maximum number of characters to read, including the null terminator.
        :return: A string containing the characters read from the stream, or an empty string if the end of
                 the stream is reached.

        **Warning**: Ensure that the buffer size is large enough to hold the expected input. Reading from an
        invalid or closed stream can result in undefined behavior.
        """
        pass
