from typing import TypeVar

Ptr = TypeVar("Ptr", bound=int)


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

    def aligned_malloc(self, __size: int, __alignment: int) -> Ptr:
        """
        Allocates a block of memory of the given size, aligned to the specified boundary.

        :param __size: The number of bytes to allocate.
        :param __alignment: The alignment boundary in bytes. Must be a power of two.
        :return: A pointer (integer representing the memory address) to the allocated and aligned memory block.

        **Warning**: The allocated memory is uninitialized. You are responsible for managing this memory
        and freeing it with `aligned_free` when no longer needed.
        """
        pass

    def aligned_free(self, __pointer: Ptr) -> None:
        """
        Frees a previously allocated block of memory that was aligned with `aligned_malloc`.

        :param __pointer: The pointer to the aligned memory block to free.

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

    def callVoid(self, __func: Ptr) -> None:
        """
        Calls a C function at the given memory address with no return value.

        :param __func: A `Ptr` representing the memory address of the C function to call.

        **Details**:
            - This method directly invokes the C function at the specified address.
            - The function being called must not require any arguments, as this method does not support argument passing.
            - If the function has a return value, it will be ignored.

        **Limitations**:
            - This method does not support passing arguments to the function. To pass arguments, you can use the
              `ASM` class to create a wrapper function with inline assembly.

        **Warning**:
            - The execution of C functions is inherently unsafe and may result in segmentation faults or undefined behavior
              if the function is invalid or improperly used.
            - Python exceptions will not be raised if the function fails or causes a crash. It is the caller's responsibility
              to ensure that the function is valid and safe to execute.
        """
        pass

    def callInt(self, __func: Ptr) -> int:
        """
        Calls a C function at the given memory address and returns an `int` result.

        :param __func: A `Ptr` representing the memory address of the C function to call.
        :return: An `int` representing the result returned by the C function.

        **Details**:
            - This method directly invokes the C function at the specified address and interprets its return value as an `int`.
            - The function being called must not require any arguments, as this method does not support argument passing.

        **Limitations**:
            - This method does not support passing arguments to the function. To pass arguments, you can use the
              `ASM` class to create a wrapper function with inline assembly.

        **Warning**:
            - The execution of C functions is inherently unsafe and may result in segmentation faults or undefined behavior
              if the function is invalid or improperly used.
            - Python exceptions will not be raised if the function fails or causes a crash. It is the caller's responsibility
              to ensure that the function is valid and safe to execute.
        """
        pass

    def callLongLong(self, __func: Ptr) -> int:
        """
        Calls a C function at the given memory address and returns a `long long` result.

        :param __func: A `Ptr` representing the memory address of the C function to call.
        :return: An `int` representing the result returned by the C function, interpreted as a `long long`.

        **Details**:
            - This method directly invokes the C function at the specified address and interprets its return value as a `long long`.
            - The function being called must not require any arguments, as this method does not support argument passing.

        **Limitations**:
            - This method does not support passing arguments to the function. To pass arguments, you can use the
              `ASM` class to create a wrapper function with inline assembly.

        **Warning**:
            - The execution of C functions is inherently unsafe and may result in segmentation faults or undefined behavior
              if the function is invalid or improperly used.
            - Python exceptions will not be raised if the function fails or causes a crash. It is the caller's responsibility
              to ensure that the function is valid and safe to execute.
        """
        pass

    def call(self, __func: Ptr, __result: Ptr, __size: int) -> None:
        """
        Calls a C function at the given memory address and writes its return value to a memory block.

        :param __func: A `Ptr` representing the memory address of the C function to call.
        :param __result: A `Ptr` representing the memory address where the return value will be written.
        :param __size: The size of the return value in bytes.

        **Behavior**:
            - If `__size == 0`, this behaves like `Unsafe.callVoid()`, and `__result` is ignored.
            - The function uses the platform's default calling convention.
            - On Windows x64:
                - Return values larger than 16 bytes are returned as pointers to memory.
                - **This method does not support handling return values between 9 and 16 bytes.** For such cases, use the `ASM` class to create a custom wrapper function.

        **Limitations**:
            - Only supports calling functions with no arguments.
            - The caller must ensure `__result` points to a memory block large enough to hold the return value.
            - On Windows x64, return values between 9 and 16 bytes cannot be handled correctly by this method.

        **Recommendation**:
            - For complex return values (e.g., structures larger than 8 bytes but smaller than 16 bytes on Windows x64), consider using the `ASM` class to create a wrapper function that processes the return value directly.

        **Warning**:
            - This method is inherently unsafe. Invalid function pointers or incorrect usage may cause crashes or undefined behavior.
            - Advanced users can set `__size` to 8 to receive a pointer for large return values and manage the memory manually.
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

    def memset(self, __address: Ptr, __val: int, __size: int) -> None:
        """
        Sets a block of memory to a specified value.

        :param __address: The pointer to the memory block to be set.
        :param __val: The value to set each byte of the memory block to (0-255).
        :param __size: The number of bytes to set.

        **Description**:
        This function initializes a memory block with a specified value. Each byte in the block of
        memory, starting from the given address, will be set to the provided value.

        **Warning**:
        - Ensure that the memory block starting at `__address` is valid and large enough to hold
          `__size` bytes. Writing to invalid memory can cause undefined behavior.
        - The value `__val` is treated as a single byte (0-255). If a value outside this range is
          provided, it will be truncated to fit within a byte.
        ```
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