import array
import timeit
import numpy
from pyfastutil.ints import IntArrayList

SIZE = int(1e7)
REPEAT = 3


def python_list():
    list(range(SIZE))


def python_array():
    array.array('i', range(SIZE))


def numpy_():
    numpy.array(range(SIZE))


def pyfastutil():
    IntArrayList(range(SIZE), SIZE)


if __name__ == '__main__':
    time_python_list = timeit.Timer(python_list).timeit(REPEAT) / REPEAT
    time_python_array = timeit.Timer(python_array).timeit(REPEAT) / REPEAT
    time_numpy = timeit.Timer(numpy_).timeit(REPEAT) / REPEAT
    time_pyfastutil = timeit.Timer(pyfastutil).timeit(REPEAT) / REPEAT

    print("Batch size:", SIZE)
    print("Repeat:", REPEAT)
    print(f"Python list init time: {time_python_list * 1000:.2f} ms")
    print(f"Python array init time: {time_python_array * 1000:.2f} ms")
    print(f"Numpy init time: {time_numpy * 1000:.2f} ms")
    print(f"PyFastUtil IntArrayList init time: {time_pyfastutil * 1000:.2f} ms")
    print()

    print(f"Python array speed of Python list: {time_python_list / time_python_array * 100:.3f} %")
    print(f"Numpy speed of Python list: {time_python_list / time_numpy * 100:.3f} %")
    print(f"PyFastUtil speed of Python list: {time_python_list / time_pyfastutil * 100:.3f} %")
