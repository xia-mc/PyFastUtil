import array
import timeit

from pyfastutil.ints import IntArrayList

SIZE = int(1e7)
REPEAT = 3


def python_list():
    lst = list()
    for i in range(SIZE):
        lst.append(i)


def python_array():
    lst = array.array("i")
    for i in range(SIZE):
        lst.append(i)


def pyfastutil():
    lst = IntArrayList()
    for i in range(SIZE):
        lst.append(i)


if __name__ == '__main__':
    time_python_list = timeit.Timer(python_list).timeit(REPEAT) / REPEAT
    time_python_array = timeit.Timer(python_array).timeit(REPEAT) / REPEAT
    time_pyfastutil = timeit.Timer(pyfastutil).timeit(REPEAT) / REPEAT

    print("Batch size:", SIZE)
    print("Repeat:", REPEAT)
    print(f"Python list append time: {time_python_list * 1000:.2f} ms")
    print(f"Python array append time: {time_python_array * 1000:.2f} ms")
    print(f"PyFastUtil append time: {time_pyfastutil * 1000:.2f} ms")
    print()

    print(f"Python array speed of Python list: {time_python_list / time_python_array * 100:.3f} %")
    print(f"PyFastUtil speed of Python list: {time_python_list / time_pyfastutil * 100:.3f} %")
