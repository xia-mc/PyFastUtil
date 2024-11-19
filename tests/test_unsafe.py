import sys
import unittest
import struct

from pyfastutil.unsafe import Unsafe, Ptr, NULL


class TestUnsafe(unittest.TestCase):

    def test_malloc_and_free(self):
        with Unsafe() as unsafe:
            ptr = unsafe.malloc(10)
            self.assertIsInstance(ptr, int)
            self.assertGreater(ptr, 0)
            unsafe.free(ptr)

    def test_aligned_malloc_and_free(self):
        with Unsafe() as unsafe:
            ptr = unsafe.aligned_malloc(10, 64)
            self.assertIsInstance(ptr, int)
            self.assertGreater(ptr, 0)
            self.assertEqual(ptr % 64, 0)
            unsafe.aligned_free(ptr)

    def test_calloc(self):
        with Unsafe() as unsafe:
            ptr = unsafe.calloc(10, 4)
            self.assertIsInstance(ptr, int)
            self.assertGreater(ptr, 0)
            data = unsafe.get(ptr, 40)
            self.assertEqual(data, b'\x00' * 40)
            unsafe.free(ptr)

    def test_realloc(self):
        with Unsafe() as unsafe:
            ptr = unsafe.malloc(10)
            self.assertGreater(ptr, 0)
            new_ptr = unsafe.realloc(ptr, 20)
            self.assertGreater(new_ptr, 0)
            self.assertNotEqual(new_ptr, NULL)
            unsafe.free(new_ptr)

    def test_set_and_get(self):
        with Unsafe() as unsafe:
            ptr = unsafe.malloc(40)
            byte_data = struct.pack('10i', 1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
            unsafe.set(ptr, byte_data)
            result = struct.unpack('i', unsafe.get(ptr + 4 * 2, 4))[0]
            self.assertEqual(result, 3)
            unsafe.free(ptr)

    def test_memcpy(self):
        with Unsafe() as unsafe:
            src_ptr = unsafe.malloc(40)
            dest_ptr = unsafe.malloc(40)
            byte_data = struct.pack('10i', 1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
            unsafe.set(src_ptr, byte_data)
            unsafe.memcpy(src_ptr, dest_ptr, 40)
            copied_data = unsafe.get(dest_ptr, 40)
            self.assertEqual(copied_data, byte_data)
            unsafe.free(src_ptr)
            unsafe.free(dest_ptr)

    def test_get_address_and_as_object(self):
        with Unsafe() as unsafe:
            obj = [1, 2, 3]
            ptr = unsafe.get_address(obj)
            self.assertIsInstance(ptr, int)
            self.assertGreater(ptr, 0)
            restored_obj = unsafe.as_object(ptr)
            self.assertEqual(restored_obj, obj)

    def test_incref_and_decref(self):
        with Unsafe() as unsafe:
            obj = [1, 2, 3]
            initial_ref_count = sys.getrefcount(obj)
            unsafe.incref(obj)
            self.assertEqual(sys.getrefcount(obj), initial_ref_count + 1)
            unsafe.decref(obj)
            self.assertEqual(sys.getrefcount(obj), initial_ref_count)

    def test_fputs_and_fflush(self):
        with Unsafe() as unsafe:
            unsafe.fputs("Hello, Unsafe World!\n")
            unsafe.fflush()


if __name__ == '__main__':
    unittest.main()
