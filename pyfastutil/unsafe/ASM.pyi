from typing import TypeVar

Ptr = TypeVar("Ptr", bound=int)


class ASM:
    """
    A class for executing raw machine instructions. This class provides methods to execute
    pre-compiled bytecode directly, enabling low-level control over the CPU. It is designed for
    advanced users who need precise control over machine-level operations.

    **Best Practice**:
        Use this class within a `with` statement to limit the scope of operations, similar to the
        concept of `unsafe` blocks in Rust. This ensures that potentially dangerous actions are
        confined to a small, controlled section of code.

        Example:
        ```python
        with ASM() as asm:
            asm.run(b"...")  # Execute machine instructions
        ```

    **Warning**:
        - The methods in this class execute arbitrary machine instructions, which can lead to crashes,
          memory corruption, or undefined behavior if used improperly.
        - The input `__code` must be valid, pre-compiled machine code. It is recommended to use
          libraries like Keystone to assemble instructions into bytecode.
        - **Executing any machine instruction inherently results in undefined or implementation-defined behavior.**
          This means the effects of the execution are highly dependent on the specific CPU, operating
          system, and runtime environment.
        - During the execution of machine instructions, **Python exceptions will not be raised**, even if
          the instructions cause severe issues. This may result in:
            - Program crashes
            - Operating system crashes
            - Hardware damage in extreme cases (e.g., executing instructions that manipulate power states
              or firmware).

    **Use with Extreme Caution**:
        This class bypasses Python's safety mechanisms and should only be used by advanced users
        who fully understand the risks of executing raw machine code. Improper use can compromise
        system stability and security.
    """

    def __init__(self) -> None:
        """
        Initializes the ASM context. This constructor does not execute any code or perform any
        operations. It simply prepares the object to be used in a `with` statement or as a standalone
        instance.
        """
        pass

    def __enter__(self) -> ASM: ...
    def __exit__(self, exc_type, exc_val, exc_tb) -> None: ...

    def run(self, __code: bytes) -> None:
        """
        Executes the provided machine code.

        :param __code: A `bytes` object containing valid, pre-compiled machine instructions.

        **Details**:
            - The provided `__code` must be a valid sequence of machine instructions for the current
              architecture.
            - It is recommended to use an assembler library like Keystone to generate the machine code.
            - Before execution, the memory containing the instructions will be marked as executable.
            - This method performs basic validation and ensures proper memory alignment before execution.

        **Warning**:
            - Improper or invalid instructions can cause crashes, undefined behavior, or security risks.
            - The execution of machine instructions is inherently unsafe and may result in system instability.
            - **No Python exceptions will be raised if the instructions fail or cause crashes.**
        """
        pass

    def runFast(self, __code: bytes) -> None:
        """
        Executes the provided machine code without performing validation or memory alignment checks.

        :param __code: A `bytes` object containing valid, pre-compiled machine instructions.

        **Details**:
            - This method is similar to `run`, but skips input validation and alignment checks for
              performance reasons.
            - The caller is responsible for ensuring that `__code` is valid, properly aligned, and
              executable.
            - Skipping these checks can improve performance but increases the risk of undefined behavior.

        **Warning**:
            - This method is inherently unsafe and should only be used when you are certain that the
              input is valid.
            - Invalid or misaligned instructions can cause crashes, undefined behavior, or security risks.
            - **No Python exceptions will be raised if the instructions fail or cause crashes.**
        """
        pass

    def makeFunction(self, __code: bytes) -> Ptr:
        """
        Creates a callable C function from the provided machine code.

        :param __code: A `bytes` object containing valid, pre-compiled machine instructions.

        :return: A `Ptr` representing the starting address of the created function. This pointer
                 can be used with `Unsafe.call`-series functions to invoke the function.

        **Details**:
            - This method allocates memory for the function, marks it as executable, and copies the
              provided machine code into the allocated memory.
            - The returned `Ptr` is a new reference and must be released using `freeFunction`
              to avoid memory leaks.
            - This method is thread-safe and performs validation on the input `__code`.
            - Do not mix `makeFunction` with `Unsafe.malloc` or `Unsafe.set`. Functions created by
              `makeFunction` must be managed exclusively with `makeFunction` and `freeFunction`.

        **Warning**:
            - Improper or invalid machine code can cause crashes, undefined behavior, or security risks.
            - The execution of machine instructions is inherently unsafe and may result in system instability.
        """
        pass

    def makeFunctionFast(self, __code: bytes) -> Ptr:
        """
        Creates a callable C function from the provided machine code, without performing validation
        or memory alignment checks.

        :param __code: A `bytes` object containing valid, pre-compiled machine instructions.

        :return: A `Ptr` representing the starting address of the created function. This pointer
                 can be used with `Unsafe.call`-series functions to invoke the function.

        **Details**:
            - This method is similar to `makeFunction`, but skips input validation and alignment checks
              for performance reasons.
            - The caller is responsible for ensuring that `__code` is valid, properly aligned, and executable.
            - The returned `Ptr` is a new reference and must be released using `freeFunctionFast`
              to avoid memory leaks.
            - This method is **not thread-safe** and should only be used in single-threaded contexts.
            - Do not mix `makeFunctionFast` with `Unsafe.malloc` or `Unsafe.set`. Functions created by
              `makeFunctionFast` must be managed exclusively with `makeFunctionFast` and `freeFunctionFast`.

        **Warning**:
            - Invalid or misaligned machine code can cause crashes, undefined behavior, or security risks.
            - This method is inherently unsafe and may result in system instability.
        """
        pass

    def freeFunction(self, __func: Ptr) -> None:
        """
        Releases the memory associated with a function created by `makeFunction`.

        :param __func: A `Ptr` representing the starting address of a function previously created
                       by `makeFunction`.

        **Details**:
            - This method frees the memory allocated for the function, ensuring no memory leaks occur.
            - The `__func` pointer must be the return value of a previous `makeFunction` call and must
              not have been released already.
            - Attempting to release an invalid or previously freed pointer will raise a `ValueError`.
            - This method is thread-safe and ensures proper synchronization when freeing resources.
            - Do not mix `freeFunction` with `Unsafe.free`. Functions created by `makeFunction` must
              be managed exclusively with `makeFunction` and `freeFunction`.

        **Warning**:
            - Passing invalid or previously freed pointers to this method can result in undefined behavior.
            - Ensure that the function pointer is not used after it has been freed.
        """
        pass

    def freeFunctionFast(self, __func: Ptr) -> None:
        """
        Releases the memory associated with a function created by `makeFunctionFast`, without
        performing validation.

        :param __func: A `Ptr` representing the starting address of a function previously created
                       by `makeFunctionFast`.

        **Details**:
            - This method is similar to `freeFunction`, but skips input validation for performance reasons.
            - The caller is responsible for ensuring that `__func` is valid and was created by
              `makeFunctionFast`.
            - This method is **not thread-safe** and should only be used in single-threaded contexts.
            - Do not mix `freeFunctionFast` with `Unsafe.free`. Functions created by `makeFunctionFast`
              must be managed exclusively with `makeFunctionFast` and `freeFunctionFast`.

        **Warning**:
            - Passing invalid or previously freed pointers to this method can result in undefined behavior.
            - This method is inherently unsafe and may result in system instability.
            - Ensure that the function pointer is not used after it has been freed.
        """
        pass
