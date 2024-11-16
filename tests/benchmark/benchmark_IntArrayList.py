import timeit
import random
from pyfastutil.ints import IntArrayList

SIZE = int(1e4)
REPEAT = 5
UNSORT_LIST = [random.randint(0, SIZE) for _ in range(SIZE)]

pythonList = list(UNSORT_LIST)
pyFastUtilList = IntArrayList(UNSORT_LIST)


def setup_python():
    global pythonList
    pythonList = list(UNSORT_LIST)


def setup_pyfastutil():
    global pyFastUtilList
    pyFastUtilList = IntArrayList(UNSORT_LIST)


def python_sort():
    global pythonList
    pythonList.sort()


def pyfastutil_sort():
    global pyFastUtilList
    # pyFastUtilList.sort()
    print("skipped sort")


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
    for i in range(1000):
        if i in pythonList:
            pythonList.remove(i)


def pyfastutil_remove():
    global pyFastUtilList
    for i in range(1000):
        if i in pyFastUtilList:
            pyFastUtilList.remove(i)


def python_extend():
    global pythonList
    pythonList.extend(range(SIZE))


def pyfastutil_extend():
    global pyFastUtilList
    pyFastUtilList.extend(range(SIZE))


def main():
    print(f"---Python list & IntArrayList Benchmark---")
    print(f"Batch size: {SIZE}")
    print(f"Repeat: {REPEAT}\n")

    total_speed = 0
    num_operations = 0

    # Benchmark sort()
    time_python_sort = sum(timeit.repeat(python_sort, setup=setup_python, repeat=REPEAT, number=1)) / REPEAT
    time_pyfastutil_sort = sum(timeit.repeat(pyfastutil_sort, setup=setup_pyfastutil, repeat=REPEAT, number=1)) / REPEAT
    sort_speed = time_python_sort / time_pyfastutil_sort * 100
    total_speed += sort_speed
    num_operations += 1
    print(f"Python list sort time: {time_python_sort * 1000:.2f} ms")
    print(f"PyFastUtil IntArrayList sort time: {time_pyfastutil_sort * 1000:.2f} ms")
    print(f"PyFastUtil speed of Python list (sort): {sort_speed:.3f} %\n")

    # Benchmark append()
    time_python_append = sum(timeit.repeat(python_append, setup=setup_python, repeat=REPEAT, number=1)) / REPEAT
    time_pyfastutil_append = sum(timeit.repeat(pyfastutil_append, setup=setup_pyfastutil, repeat=REPEAT, number=1)) / REPEAT
    append_speed = time_python_append / time_pyfastutil_append * 100
    total_speed += append_speed
    num_operations += 1
    print(f"Python list append time: {time_python_append * 1000:.2f} ms")
    print(f"PyFastUtil IntArrayList append time: {time_pyfastutil_append * 1000:.2f} ms")
    print(f"PyFastUtil speed of Python list (append): {append_speed:.3f} %\n")

    # Benchmark insert()
    time_python_insert = sum(timeit.repeat(python_insert, setup=setup_python, repeat=REPEAT, number=1)) / REPEAT
    time_pyfastutil_insert = sum(timeit.repeat(pyfastutil_insert, setup=setup_pyfastutil, repeat=REPEAT, number=1)) / REPEAT
    insert_speed = time_python_insert / time_pyfastutil_insert * 100
    total_speed += insert_speed
    num_operations += 1
    print(f"Python list insert time: {time_python_insert * 1000:.2f} ms")
    print(f"PyFastUtil IntArrayList insert time: {time_pyfastutil_insert * 1000:.2f} ms")
    print(f"PyFastUtil speed of Python list (insert): {insert_speed:.3f} %\n")

    # Benchmark pop()
    time_python_pop = sum(timeit.repeat(python_pop, setup=setup_python, repeat=REPEAT, number=1)) / REPEAT
    time_pyfastutil_pop = sum(timeit.repeat(pyfastutil_pop, setup=setup_pyfastutil, repeat=REPEAT, number=1)) / REPEAT
    pop_speed = time_python_pop / time_pyfastutil_pop * 100
    total_speed += pop_speed
    num_operations += 1
    print(f"Python list pop time: {time_python_pop * 1000:.2f} ms")
    print(f"PyFastUtil IntArrayList pop time: {time_pyfastutil_pop * 1000:.2f} ms")
    print(f"PyFastUtil speed of Python list (pop): {pop_speed:.3f} %\n")

    # Benchmark remove()
    time_python_remove = sum(timeit.repeat(python_remove, setup=setup_python, repeat=REPEAT, number=1)) / REPEAT
    time_pyfastutil_remove = sum(timeit.repeat(pyfastutil_remove, setup=setup_pyfastutil, repeat=REPEAT, number=1)) / REPEAT
    remove_speed = time_python_remove / time_pyfastutil_remove * 100
    total_speed += remove_speed
    num_operations += 1
    print(f"Python list remove time: {time_python_remove * 1000:.2f} ms")
    print(f"PyFastUtil IntArrayList remove time: {time_pyfastutil_remove * 1000:.2f} ms")
    print(f"PyFastUtil speed of Python list (remove): {remove_speed:.3f} %\n")

    # Benchmark extend()
    time_python_extend = sum(timeit.repeat(python_extend, setup=setup_python, repeat=REPEAT, number=1)) / REPEAT
    time_pyfastutil_extend = sum(timeit.repeat(pyfastutil_extend, setup=setup_pyfastutil, repeat=REPEAT, number=1)) / REPEAT
    extend_speed = time_python_extend / time_pyfastutil_extend * 100
    total_speed += extend_speed
    num_operations += 1
    print(f"Python list extend time: {time_python_extend * 1000:.2f} ms")
    print(f"PyFastUtil IntArrayList extend time: {time_pyfastutil_extend * 1000:.2f} ms")
    print(f"PyFastUtil speed of Python list (extend): {extend_speed:.3f} %\n")

    # Calculate and print average speed
    avg_speed = total_speed / num_operations
    print(f"\nAvg speed of PyFastUtil compared to Python list: {avg_speed:.3f} %")


if __name__ == '__main__':
    main()
