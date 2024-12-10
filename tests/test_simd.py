import unittest
import ctypes
from typing import Callable, Iterable

from pyfastutil.unsafe import SIMD, Unsafe, Ptr, NULL

FUNCTIONS = [
    ("Int", ctypes.c_int),
    ("UnsignedInt", ctypes.c_uint),
    ("Long", ctypes.c_long),
    ("UnsignedLong", ctypes.c_ulong),
    ("LongLong", ctypes.c_longlong),
    ("UnsignedLongLong", ctypes.c_ulonglong),
    ("Short", ctypes.c_short),
    ("UnsignedShort", ctypes.c_ushort),
    ("Float", ctypes.c_float),
    ("Double", ctypes.c_double),
    ("LongDouble", ctypes.c_longdouble),
    ("Char", ctypes.c_char),
    ("UnsignedChar", ctypes.c_ubyte),
    ("WChar", ctypes.c_wchar),
    ("Char16", ctypes.c_uint16),  # UTF-16 (16-bit)
    ("Char32", ctypes.c_uint32),  # UTF-32 (32-bit)
    ("Bool", ctypes.c_bool),
    ("Int8", ctypes.c_int8),
    ("UInt8", ctypes.c_uint8),
    ("Int16", ctypes.c_int16),
    ("UInt16", ctypes.c_uint16),
    ("Int32", ctypes.c_int32),
    ("UInt32", ctypes.c_uint32),
    ("Int64", ctypes.c_int64),
    ("UInt64", ctypes.c_uint64),
    ("VoidPtr", ctypes.c_void_p),
    ("IntPtr", ctypes.POINTER(ctypes.c_int)),
    ("FloatPtr", ctypes.POINTER(ctypes.c_float)),
    ("PyObjectPtr", ctypes.py_object)
]


class TestSIMD(unittest.TestCase):
    def test_AVX512SupportCheck(self):
        with SIMD() as simd:
            self.assertTrue(isinstance(simd.isAVX512Supported(), bool))

    def test_AVX2SupportCheck(self):
        with SIMD() as simd:
            self.assertTrue(isinstance(simd.isAVX2Supported(), bool))

    def test_SSE41SupportCheck(self):
        with SIMD() as simd:
            self.assertTrue(isinstance(simd.isSSE41Supported(), bool))

    def test_SSSE3SupportCheck(self):
        with SIMD() as simd:
            self.assertTrue(isinstance(simd.isSSSE3Supported(), bool))

    def test_ArmNeonSupportCheck(self):
        with SIMD() as simd:
            self.assertTrue(isinstance(simd.isArmNeonSupported(), bool))

    @staticmethod
    def byteSum(data: Iterable[bytes]) -> bytes:
        result = b""
        for byte in data:
            result += byte
        return result

    def test_setAVX512Vector32(self):
        with Unsafe() as unsafe, SIMD() as simd:
            if not simd.isAVX512Supported():
                self.skipTest("AVX512 is unsupported.")
            vec512i = unsafe.aligned_malloc(64, 64)
            simd.setAVX512Vector32(vec512i, *range(16))
            self.assertEqual(unsafe.get(vec512i, 64), self.byteSum(bytes(ctypes.c_int(i)) for i in reversed(range(16))))
            unsafe.aligned_free(vec512i)
            del vec512i

    def test_setAVX512Vector16(self):
        with Unsafe() as unsafe, SIMD() as simd:
            if not simd.isAVX512Supported():
                self.skipTest("AVX512 is unsupported.")
            vec512i = unsafe.aligned_malloc(64, 64)
            simd.setAVX512Vector16(vec512i, *range(32))
            self.assertEqual(unsafe.get(vec512i, 64), self.byteSum(bytes(ctypes.c_short(i)) for i in reversed(range(32))))
            unsafe.aligned_free(vec512i)
            del vec512i

    def test_setAVX512Vector8(self):
        with Unsafe() as unsafe, SIMD() as simd:
            if not simd.isAVX512Supported():
                self.skipTest("AVX512 is unsupported.")
            vec512i = unsafe.aligned_malloc(64, 64)
            simd.setAVX512Vector8(vec512i, *range(64))
            self.assertEqual(unsafe.get(vec512i, 64), self.byteSum(bytes(ctypes.c_char(i)) for i in reversed(range(64))))
            unsafe.aligned_free(vec512i)
            del vec512i

    def test_setAVX2Vector32(self):
        with Unsafe() as unsafe, SIMD() as simd:
            if not simd.isAVX2Supported():
                self.skipTest("AVX2 is unsupported.")
            vec256i = unsafe.aligned_malloc(32, 32)
            simd.setAVX2Vector32(vec256i, *range(8))
            self.assertEqual(unsafe.get(vec256i, 32), self.byteSum(bytes(ctypes.c_int(i)) for i in reversed(range(8))))
            unsafe.aligned_free(vec256i)
            del vec256i

    def test_setAVX2Vector16(self):
        with Unsafe() as unsafe, SIMD() as simd:
            if not simd.isAVX2Supported():
                self.skipTest("AVX2 is unsupported.")
            vec256i = unsafe.aligned_malloc(32, 32)
            simd.setAVX2Vector16(vec256i, *range(16))
            self.assertEqual(unsafe.get(vec256i, 32), self.byteSum(bytes(ctypes.c_short(i)) for i in reversed(range(16))))
            unsafe.aligned_free(vec256i)
            del vec256i

    def test_setAVX2Vector8(self):
        with Unsafe() as unsafe, SIMD() as simd:
            if not simd.isAVX2Supported():
                self.skipTest("AVX2 is unsupported.")
            vec256i = unsafe.aligned_malloc(32, 32)
            simd.setAVX2Vector8(vec256i, *range(32))
            self.assertEqual(unsafe.get(vec256i, 32), self.byteSum(bytes(ctypes.c_char(i)) for i in reversed(range(32))))
            unsafe.aligned_free(vec256i)
            del vec256i


def initTestCases():
    COUNT = 16
    VALUE = 1

    for name, ctype in FUNCTIONS:
        size = ctypes.sizeof(ctype)
        fieldSize = size * COUNT

        def testMemcpy(self_: unittest.TestCase):
            with Unsafe() as unsafe, SIMD() as simd:
                srcPtr = unsafe.malloc(fieldSize)
                dstPtr = unsafe.malloc(fieldSize)
                data = bytes(ctype(VALUE)) * COUNT

                unsafe.set(srcPtr, data)
                self_.assertEqual(unsafe.get(srcPtr, fieldSize), data)

                func: Callable[[Ptr, Ptr, int], None] = getattr(simd, f"memcpy{name}")
                func(srcPtr, dstPtr, COUNT)

                self_.assertEqual(unsafe.get(dstPtr, fieldSize), data)

                unsafe.free(srcPtr)
                unsafe.free(dstPtr)
                del srcPtr, dstPtr

        def testMemcpyAligned(self_: unittest.TestCase):
            with Unsafe() as unsafe, SIMD() as simd:
                srcPtr = unsafe.aligned_malloc(fieldSize, 64)
                dstPtr = unsafe.aligned_malloc(fieldSize, 64)
                self_.assertEqual(srcPtr % 64, 0)
                self_.assertEqual(dstPtr % 64, 0)

                data = bytes(ctype(VALUE)) * COUNT

                unsafe.set(srcPtr, data)
                self_.assertEqual(unsafe.get(srcPtr, fieldSize), data)

                func: Callable[[Ptr, Ptr, int], None] = getattr(simd, f"memcpy{name}Aligned")
                func(srcPtr, dstPtr, COUNT)

                self_.assertEqual(unsafe.get(dstPtr, fieldSize), data)

                unsafe.aligned_free(srcPtr)
                unsafe.aligned_free(dstPtr)
                del srcPtr, dstPtr

        setattr(TestSIMD, f"test_memCpy{name}", testMemcpy)
        setattr(TestSIMD, f"test_memCpy{name}Aligned", testMemcpyAligned)


initTestCases()

if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(TestSIMD)
    unittest.TextTestRunner().run(suite)
