import timeit

from pyfastutil.native import native

SIZE = 1000000
REPEAT = 10


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


def main():
    global SIZE
    print(f"---Python & Native for Benchmark---")
    print(f"Batch size: {SIZE}")
    print(f"Repeat: {REPEAT}\n")

    assert python(SIZE) == native(SIZE)
    assert native.__doc__ == "<native method>"

    time_python = sum(timeit.repeat(lambda: python(SIZE), repeat=REPEAT, number=1)) / REPEAT
    time_native = sum(timeit.repeat(lambda: native(SIZE), repeat=REPEAT, number=1)) / REPEAT
    speed = time_python / time_native * 100

    print(f"Python time: {time_python * 1000:.2f} ms")
    print(f"Native time: {time_native * 1000:.2f} ms")
    print(f"Native speed of Python: {speed:.2f} %\n")


if __name__ == '__main__':
    main()
