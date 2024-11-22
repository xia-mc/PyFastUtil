import sys

sys.path.append('/')

import timeit
import numpy
import random
from pyfastutil.ints import IntArrayList

SIZE = int(1e7)
REPEAT = 3
UNSORT_LIST = [random.randint(0, SIZE) for i in range(SIZE)]

pythonList = list(UNSORT_LIST)
numpyList = numpy.array(UNSORT_LIST)
pyFastUtilList = IntArrayList(UNSORT_LIST)


def python():
    pythonList.copy()


def numpy_():
    numpyList.copy()


def pyfastutil():
    pyFastUtilList.copy()


if __name__ == '__main__':
    time_python_list = sum(timeit.repeat(python, repeat=REPEAT, number=1)) / REPEAT
    time_numpy = sum(timeit.repeat(numpy_, repeat=REPEAT, number=1)) / REPEAT
    time_pyfastutil = sum(timeit.repeat(pyfastutil, repeat=REPEAT, number=1)) / REPEAT

    print("Batch size:", SIZE)
    print("Repeat:", REPEAT)
    print(f"Python list copy time: {time_python_list * 1000:.2f} ms")
    print(f"Numpy copy time: {time_numpy * 1000:.2f} ms")
    print(f"PyFastUtil IntArrayList copy time: {time_pyfastutil * 1000:.2f} ms")
    print()

    print(f"Numpy speed of Python list: {time_python_list / time_numpy * 100:.3f} %")
    print(f"PyFastUtil speed of Python list: {time_python_list / time_pyfastutil * 100:.3f} %")
