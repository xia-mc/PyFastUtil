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


def setup_python():
    global pythonList
    pythonList = list(UNSORT_LIST)


def setup_numpy():
    global numpyList
    numpyList = numpy.array(UNSORT_LIST)


def setup_pyfastutil():
    global pyFastUtilList
    pyFastUtilList = IntArrayList(UNSORT_LIST)


def python_list():
    global pythonList
    pythonList.sort()


def numpy_():
    global numpyList
    numpyList.sort()


def pyfastutil():
    global pyFastUtilList
    pyFastUtilList.sort()


if __name__ == '__main__':
    time_pyfastutil = timeit.Timer(pyfastutil, setup=setup_pyfastutil).timeit(REPEAT) / REPEAT
    time_python_list = timeit.Timer(python_list, setup=setup_python).timeit(REPEAT) / REPEAT
    time_numpy = timeit.Timer(numpy_, setup=setup_numpy).timeit(REPEAT) / REPEAT

    print("Batch size:", SIZE)
    print("Repeat:", REPEAT)
    print(f"Python list sort time: {time_python_list * 1000:.2f} ms")
    print(f"Numpy sort time: {time_numpy * 1000:.2f} ms")
    print(f"PyFastUtil IntArrayList sort time: {time_pyfastutil * 1000:.2f} ms")
    print()

    print(f"Numpy speed of Python list: {time_python_list / time_numpy * 100:.3f} %")
    print(f"PyFastUtil speed of Python list: {time_python_list / time_pyfastutil * 100:.3f} %")
