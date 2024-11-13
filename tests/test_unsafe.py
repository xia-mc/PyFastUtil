import time
import unittest
import sys
import threading
import timeit

from pyfastutil.unsafe import Unsafe


class MyTestCase(unittest.TestCase):
    def test_malloc(self):
        BYTE = b'a'
        with Unsafe() as unsafe:
            ptr = unsafe.malloc(1)
            unsafe.set(ptr, BYTE)
            self.assertEqual(unsafe.get(ptr, 1), BYTE)  # add assertion here
            unsafe.free(ptr)

    def test_memcpy(self):
        obj = "hello"
        size = sys.getsizeof(obj)
        with Unsafe() as unsafe:
            ptr1 = unsafe.get_address(obj)
            ptr2 = unsafe.malloc(size)
            unsafe.memcpy(ptr1, ptr2, size)
            obj2 = unsafe.as_object(ptr2)
            self.assertEqual(obj, obj2)
            self.assertNotEqual(id(obj), id(obj2))
            unsafe.free(ptr2)

    def test_ref(self):
        obj = [1, 2, 3]
        with Unsafe() as unsafe:
            ptr = unsafe.get_address(obj)
            obj2 = unsafe.as_object(ptr)
            self.assertEqual(obj, obj2)
            self.assertEqual(id(obj), id(obj2))

    def test_multithread(self):
        def cpuBoundActions():
            unsafe.fputs("")

        def doMultiThread():
            threads = []
            for _ in range(8):
                thread = threading.Thread(target=cpuBoundActions)
                threads.append(thread)
                thread.start()

            for thread in threads:
                thread.join()

        def doMultiThreadNoGil():
            threads = []
            for _ in range(8):
                thread = threading.Thread(target=cpuBoundActions)
                threads.append(thread)
                thread.start()

            for thread in threads:
                thread.join()

        with Unsafe() as unsafe:
            withGil = timeit.Timer(doMultiThread).timeit(3) / 3
            noGil = timeit.Timer(doMultiThreadNoGil).timeit(3) / 3

            self.assertLess(noGil, withGil)


if __name__ == '__main__':
    unittest.main()
