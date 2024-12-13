from typing import Optional

from keystone import Ks, KS_ARCH_X86, KS_MODE_64
from pyfastutil.unsafe import ASM, Unsafe, Ptr

import timeit

SIZE = int(1e4)
REPEAT = 200


# Initialize Keystone assembler for amd64
ks = Ks(KS_ARCH_X86, KS_MODE_64)
asmFunc: Optional[Ptr] = None


def pythonFor():
    x = 0
    for i in range(SIZE):
        x += i
    return x


def setupAsm():
    global asmFunc

    asmCode = f"""
        mov rax, 0      # x = 0
        mov rcx, 0      # i = 0
        for:
        cmp rcx, {SIZE} # if i >= {SIZE}
        jnl end
        add rax, rcx    # x += i
        inc rcx         # i += 1
        jmp for

        end:
        ret
    """

    with ASM() as asm:
        if asmFunc is not None:
            asm.freeFunction(asmFunc)
        asmFunc = asm.makeFunction(ks.asm(asmCode, as_bytes=True)[0])


def asmFor():
    with Unsafe() as unsafe:
        return unsafe.callInt(asmFunc)


def main():
    global SIZE
    print(f"---Python for & ASM for Benchmark---")
    print(f"Repeat: {REPEAT}\n")

    for size in [1, 10, 100, 1000, 10000]:
        SIZE = size
        print(f"Batch size: {SIZE}")

        setupAsm()
        assert pythonFor() == asmFor()

        time_python = sum(timeit.repeat(pythonFor, repeat=REPEAT, number=1)) / REPEAT
        time_asm = sum(timeit.repeat(asmFor, repeat=REPEAT, number=1)) / REPEAT
        speed = time_python / time_asm * 100

        print(f"Python time: {time_python * 1000:.2f} ms")
        print(f"ASM time: {time_asm * 1000:.2f} ms")
        print(f"ASM speed of Python: {speed:.2f} %\n")

    with ASM() as asm:
        asm.freeFunction(asmFunc)


if __name__ == '__main__':
    main()
