import sys
sys.path.append("D:\\PyFastUtil")
from typing import Optional
from keystone import Ks, KS_ARCH_X86, KS_MODE_64
from pyfastutil.unsafe import Ptr, Unsafe, ASM
import timeit
from pyfastutil.native import native

SIZE = 2000
REPEAT = 20

ks = Ks(KS_ARCH_X86, KS_MODE_64)
asmFunc: Optional[Ptr] = None


def python(n):
    x = 0
    for i in range(n):
        x += i * i
    return x


@native
def native(n):
    x = 0
    for i in range(n):
        x += i * i
    return x


def setupRealNative():
    global asmFunc

    asmCode = f"""
        mov rax, 0      # x = 0
        mov r10, 0      # i = 0
        for:
        cmp r10, {SIZE} # if i >= {SIZE}
        jnl end
        mov r11, r10    # i * i
        imul r11, r11
        add rax, r11    # x += i * i
        inc r10         # i += 1
        jmp for

        end:
        ret
    """

    with ASM() as asm:
        asmFunc = asm.makeFunction(ks.asm(asmCode, as_bytes=True)[0])


def realNative():
    with Unsafe() as unsafe:
        return unsafe.callLongLong(asmFunc)














def main():
    global SIZE
    print(f"---Python & Native & Real-Native for Benchmark---")
    print(f"Batch size: {SIZE}")
    print(f"Repeat: {REPEAT}\n")

    setupRealNative()
    assert python(SIZE) == native(SIZE) == realNative()
    assert native.__doc__ == "<native method>"

    time_python = sum(timeit.repeat(lambda: python(SIZE), repeat=REPEAT, number=1)) / REPEAT
    time_native = sum(timeit.repeat(lambda: native(SIZE), repeat=REPEAT, number=1)) / REPEAT
    time_realNative = sum(timeit.repeat(realNative, repeat=REPEAT, number=1)) / REPEAT
    speed = time_python / time_native * 100
    speed2 = time_python / time_realNative * 100

    print(f"Python time: {time_python * 1000:.2f} ms")
    print(f"Native time: {time_native * 1000:.2f} ms")
    print(f"Real-Native time: {time_realNative * 1000:.2f} ms\n")
    print(f"Native speed of Python: {speed:.2f} %")
    print(f"Real-Native speed of Python: {speed2:.2f} %\n")

    with ASM() as asm:
        asm.freeFunction(asmFunc)


if __name__ == '__main__':
    main()
