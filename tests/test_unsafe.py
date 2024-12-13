import ctypes
import struct
import sys
import unittest

from keystone import Ks, KS_ARCH_X86, KS_MODE_64

from pyfastutil.unsafe import Unsafe, NULL, ASM


class TestUnsafe(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """
        Set up resources for the test class. This method runs once before all tests.
        """
        # Initialize Keystone assembler for x86-64
        cls.ks = Ks(KS_ARCH_X86, KS_MODE_64)

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

    def test_callVoid(self):
        with ASM() as asm, Unsafe() as unsafe:
            voidFunc = asm.makeFunction(self.ks.asm("ret", as_bytes=True)[0])

            unsafe.callVoid(voidFunc)

            asm.freeFunction(voidFunc)

    def test_callInt(self):
        with ASM() as asm, Unsafe() as unsafe:
            func = asm.makeFunction(self.ks.asm("mov eax, 123456; ret", as_bytes=True)[0])

            self.assertEqual(unsafe.callInt(func), 123456)

            asm.freeFunction(func)

    def test_callLongLong(self):
        with ASM() as asm, Unsafe() as unsafe:
            func = asm.makeFunction(self.ks.asm("mov rax, 12345678910; ret", as_bytes=True)[0])

            self.assertEqual(unsafe.callLongLong(func), 12345678910)

            asm.freeFunction(func)

    def test_callCall(self):
        with ASM() as asm, Unsafe() as unsafe:
            func = asm.makeFunction(self.ks.asm("mov rax, 12345678910; ret", as_bytes=True)[0])

            callResult = unsafe.malloc(8)
            unsafe.call(func, callResult, 8)
            self.assertEqual(unsafe.get(callResult, 8), bytes(ctypes.c_longlong(12345678910)))
            unsafe.free(callResult)

            asm.freeFunction(func)

    def test_call_16bytes(self):
        with ASM() as asm, Unsafe() as unsafe:
            funcRet16Bytes = asm.makeFunction(self.ks.asm("""
                mov rax, 123456
                mov rdx, 123456
                ret
            """, as_bytes=True)[0])
            funcProxy = asm.makeFunction(self.ks.asm(f"""
                push rcx
                mov rcx, {hex(funcRet16Bytes)}
                call rcx
                imul rax, rdx
                pop rcx
                ret
            """, as_bytes=True)[0])

            self.assertEqual(unsafe.callLongLong(funcProxy), 123456 * 123456)

            asm.freeFunction(funcRet16Bytes)
            asm.freeFunction(funcProxy)

    def test_call_size_zero(self):
        with ASM() as asm, Unsafe() as unsafe:
            voidFunc = asm.makeFunction(self.ks.asm("ret", as_bytes=True)[0])

            # size == 0, should behave like callVoid, pass NULL as result pointer
            unsafe.call(voidFunc, NULL, 0)

            asm.freeFunction(voidFunc)

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

    def test_memset(self):
        with Unsafe() as unsafe:
            ptr = unsafe.malloc(10)
            unsafe.memset(ptr, 1, 10)
            self.assertEqual(unsafe.get(ptr, 10), b'\x01' * 10)

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
