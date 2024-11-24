import ctypes
import timeit
import random

from pyfastutil.ints import IntArrayList, BigIntArrayList, IntLinkedList
from typing import Callable, Optional

SIZE = int(1e4)
REPEAT = 3

pyFastUtilListObj: type[list]
sortKey: Optional[Callable[[...], str]] = None

unsortList: list
pythonList: list
pyFastUtilList: list


# Setup functions to reset the lists
def setup_python():
    global pythonList
    pythonList = list(unsortList)


def setup_pyfastutil():
    global pyFastUtilList
    pyFastUtilList = pyFastUtilListObj(unsortList)


# Benchmark functions
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
    for i in range(SIZE):
        pythonList.insert(0, i)


def pyfastutil_insert():
    global pyFastUtilList
    for i in range(SIZE):
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
    for i in unsortList:
        pythonList.remove(i)


def pyfastutil_remove():
    global pyFastUtilList
    for i in unsortList:
        pyFastUtilList.remove(i)


def python_index():
    global pythonList
    for i in unsortList:
        pythonList.index(i)


def pyfastutil_index():
    global pyFastUtilList
    for i in unsortList:
        pyFastUtilList.index(i)


# This version of contains mixes present and absent elements
def python_contains():
    global pythonList
    for i in range(SIZE):
        pythonList.__contains__(random.choice([random.randint(0, SIZE), random.choice(unsortList)]))


def pyfastutil_contains():
    global pyFastUtilList
    for i in range(SIZE):
        pyFastUtilList.__contains__(random.choice([random.randint(0, SIZE), random.choice(unsortList)]))


def python_extend():
    global pythonList
    pythonList.extend(range(SIZE))


def pyfastutil_extend():
    global pyFastUtilList
    pyFastUtilList.extend(range(SIZE))


# Main benchmarking function
def main(obj: type[list]):
    global pyFastUtilListObj
    global unsortList
    global sortKey

    pyFastUtilListObj = obj

    if obj is IntArrayList or obj is IntLinkedList:
        INT_MAX = (2 ** (ctypes.sizeof(ctypes.c_int) * 8 - 1)) - 1
        INT_MIN = -INT_MAX - 1
        unsortList = [random.randint(INT_MIN, INT_MAX) for _ in range(SIZE)]
    elif obj is BigIntArrayList:
        LONG_LONG_MAX = (2 ** (ctypes.sizeof(ctypes.c_longlong) * 8 - 1)) - 1
        LONG_LONG_MIN = -LONG_LONG_MAX - 1
        unsortList = [random.randint(LONG_LONG_MIN, LONG_LONG_MAX) for _ in range(SIZE)]
    else:
        unsortList = [random.choice([str, int, float, bytes])(random.randint(0, SIZE)) for _ in range(SIZE)]
        sortKey = str

    print(f"---Python list & {pyFastUtilListObj.__name__} Benchmark---")
    print(f"Batch size: {SIZE}")
    print(f"Repeat: {REPEAT}\n")

    total_speed = 0
    num_operations = 0

    # List of benchmark operations
    benchmarks = [
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
    main(IntArrayList)
