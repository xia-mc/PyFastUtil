import ctypes
import timeit
import random

from pyfastutil.ints import *
from pyfastutil.objects import *
from typing import Callable, Optional


SIZE = int(1e4)
REPEAT = 3

pyFastUtilListObj: type[IntArrayList] = IntArrayList
sortKey: Optional[Callable[[...], str]] = None

testData: list
randomData: list
randomIndices: list[int]

pythonList: list
# noinspection PyUnboundLocalVariable
pyFastUtilList: pyFastUtilListObj


# Setup functions to reset the lists
def setup_python():
    global pythonList
    pythonList = list(testData)


def setup_pyfastutil():
    global pyFastUtilList
    pyFastUtilList = pyFastUtilListObj(testData)


# Benchmark functions
def python_copy():
    global pythonList
    pythonList.copy()


def pyfastutil_copy():
    global pyFastUtilList
    pyFastUtilList.copy()


def python_to_python():
    global pythonList
    list(pythonList)


def pyfastutil_to_python():
    global pyFastUtilList
    pyFastUtilList.to_list()


def python_sequential_access():
    global pythonList
    for i in range(0, SIZE, -1):
        _ = pythonList[i]


def pyfastutil_sequential_access():
    global pyFastUtilList
    for i in range(0, SIZE, -1):
        _ = pyFastUtilList[i]


def python_random_access():
    global pythonList
    for i in randomIndices:
        _ = pythonList[i]


def pyfastutil_random_access():
    global pyFastUtilList
    for i in randomIndices:
        _ = pyFastUtilList[i]


def python_sort():
    global pythonList
    pythonList.sort(key=sortKey)


def pyfastutil_sort():
    global pyFastUtilList
    pyFastUtilList.sort(key=sortKey)


def python_append():
    global pythonList
    for i in range(SIZE):
        pythonList.append(i)


def pyfastutil_append():
    global pyFastUtilList
    for i in range(SIZE):
        pyFastUtilList.append(i)


def python_insert():
    global pythonList
    for i in range(10000):
        pythonList.insert(0, i)


def pyfastutil_insert():
    global pyFastUtilList
    for i in range(10000):
        pyFastUtilList.insert(0, i)


def python_pop():
    global pythonList
    for _ in range(SIZE):
        pythonList.pop()


def pyfastutil_pop():
    global pyFastUtilList
    for _ in range(SIZE):
        pyFastUtilList.pop()


def python_remove():
    global pythonList
    for i in testData:
        pythonList.remove(i)


def pyfastutil_remove():
    global pyFastUtilList
    for i in testData:
        pyFastUtilList.remove(i)


def python_index():
    global pythonList
    for i in testData:
        pythonList.index(i)


def pyfastutil_index():
    global pyFastUtilList
    for i in testData:
        pyFastUtilList.index(i)


# This version of contains mixes present and absent elements
def python_contains():
    global pythonList
    for i in randomData:
        pythonList.__contains__(i)


def pyfastutil_contains():
    global pyFastUtilList
    for i in randomData:
        pyFastUtilList.__contains__(i)


def python_extend():
    global pythonList
    pythonList.extend(range(SIZE))


def pyfastutil_extend():
    global pyFastUtilList
    pyFastUtilList.extend(range(SIZE))


# Main benchmarking function
def main(obj: type[list]):
    global pyFastUtilListObj
    global testData
    global randomData
    global randomIndices
    global sortKey

    pyFastUtilListObj = obj
    print("Preparing data...")

    if obj is IntArrayList or obj is IntLinkedList:
        INT_MAX = (2 ** (ctypes.sizeof(ctypes.c_int) * 8 - 1)) - 1
        INT_MIN = -INT_MAX - 1
        testData = [random.randint(INT_MIN, INT_MAX) for _ in range(SIZE)]
    elif obj is BigIntArrayList:
        LONG_LONG_MAX = (2 ** (ctypes.sizeof(ctypes.c_longlong) * 8 - 1)) - 1
        LONG_LONG_MIN = -LONG_LONG_MAX - 1
        testData = [random.randint(LONG_LONG_MIN, LONG_LONG_MAX) for _ in range(SIZE)]
    else:
        testData = [random.choice([str, int, float, bytes])(random.randint(0, 100000)) for _ in range(SIZE)]
        sortKey = str

    randomData = random.choices(testData + list(range(-SIZE, 0)), k=int(SIZE / 2))
    randomIndices = [random.randint(0, SIZE - 1) for _ in range(SIZE)]

    print(f"---Python list & {pyFastUtilListObj.__name__} Benchmark---")
    print(f"Batch size: {SIZE}")
    print(f"Repeat: {REPEAT}\n")

    total_speed = 0
    num_operations = 0

    # List of benchmark operations
    benchmarks = [
        ("init", setup_python, setup_pyfastutil),
        ("copy", python_copy, pyfastutil_copy),
        ("to_python", python_to_python, pyfastutil_to_python),
        ("sequential_access", python_sequential_access, pyfastutil_sequential_access),
        ("random_access", python_random_access, pyfastutil_random_access),
        ("sort", python_sort, pyfastutil_sort),
        ("append", python_append, pyfastutil_append),
        ("insert", python_insert, pyfastutil_insert),
        ("pop", python_pop, pyfastutil_pop),
        ("remove", python_remove, pyfastutil_remove),
        ("contains", python_contains, pyfastutil_contains),
        ("index", python_index, pyfastutil_index),
        ("extend", python_extend, pyfastutil_extend)
    ]

    for name, python_func, pyfastutil_func in benchmarks:
        time_python = sum(timeit.repeat(python_func, setup=setup_python, repeat=REPEAT, number=1)) / REPEAT
        time_pyfastutil = sum(timeit.repeat(pyfastutil_func, setup=setup_pyfastutil, repeat=REPEAT, number=1)) / REPEAT
        speed = time_python / time_pyfastutil * 100
        total_speed += speed
        num_operations += 1

        print(f"Python list {name} time: {time_python * 1000:.2f} ms")
        print(f"PyFastUtil {pyFastUtilListObj.__name__} {name} time: {time_pyfastutil * 1000:.2f} ms")
        print(f"PyFastUtil speed of Python list ({name}): {speed:.3f} %\n")

    # Calculate and print average speed
    avg_speed = total_speed / num_operations
    print(f"\nAvg speed of PyFastUtil compared to Python list: {avg_speed:.3f} %")


if __name__ == '__main__':
    main(ObjectArrayList)
