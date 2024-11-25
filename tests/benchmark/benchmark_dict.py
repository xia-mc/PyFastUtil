import ctypes
import timeit
import random

from pyfastutil.ints import IntIntHashMap

SIZE = int(1e5)
REPEAT = 3

pyFastUtilMapObj: type[dict]

testData: dict
pythonMap: dict
pyFastUtilMap: dict


# Setup functions to reset the dicts
def setup_python():
    global pythonMap
    pythonMap = dict(testData)


def setup_pyfastutil():
    global pyFastUtilMap
    pyFastUtilMap = pyFastUtilMapObj(testData)


# Benchmark functions
def python_copy():
    global pythonMap
    pythonMap.copy()


def pyfastutil_copy():
    global pyFastUtilMap
    pyFastUtilMap.copy()


# Main benchmarking function
def main(obj: type[dict]):
    global pyFastUtilMapObj
    global testData

    pyFastUtilMapObj = obj

    if obj is IntIntHashMap:
        INT_MAX = (2 ** (ctypes.sizeof(ctypes.c_int) * 8 - 1)) - 1
        INT_MIN = -INT_MAX - 1
        testData = {random.randint(INT_MIN, INT_MAX): random.randint(INT_MIN, INT_MAX) for _ in range(SIZE)}
    else:
        testData = {random.choice([str, int, float, bytes])(random.randint(0, 100000)):
                    random.choice([str, int, float, bytes])(random.randint(0, 100000)) for _ in range(SIZE)}

    print(f"---Python dict & {pyFastUtilMapObj.__name__} Benchmark---")
    print(f"Batch size: {SIZE}")
    print(f"Repeat: {REPEAT}\n")

    total_speed = 0
    num_operations = 0

    # Map of benchmark operations
    benchmarks = [
        ("init", setup_python, setup_pyfastutil),
        ("copy", python_copy, pyfastutil_copy)
    ]

    for name, python_func, pyfastutil_func in benchmarks:
        time_python = sum(timeit.repeat(python_func, setup=setup_python, repeat=REPEAT, number=1)) / REPEAT
        time_pyfastutil = sum(timeit.repeat(pyfastutil_func, setup=setup_pyfastutil, repeat=REPEAT, number=1)) / REPEAT
        speed = time_python / time_pyfastutil * 100
        total_speed += speed
        num_operations += 1

        print(f"Python dict {name} time: {time_python * 1000:.2f} ms")
        print(f"PyFastUtil {pyFastUtilMapObj.__name__} {name} time: {time_pyfastutil * 1000:.2f} ms")
        print(f"PyFastUtil speed of Python dict ({name}): {speed:.3f} %\n")

    # Calculate and print average speed
    avg_speed = total_speed / num_operations
    print(f"\nAvg speed of PyFastUtil compared to Python dict: {avg_speed:.3f} %")


if __name__ == '__main__':
    main(IntIntHashMap)
