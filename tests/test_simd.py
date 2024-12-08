import unittest
import ctypes
from typing import Callable

from pyfastutil.unsafe import SIMD, Unsafe, Ptr

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

        def testMemcpyAligned(self_: unittest.TestCase):
            with Unsafe() as unsafe, SIMD() as simd:
                srcPtr = unsafe.malloc(fieldSize)
                dstPtr = unsafe.malloc(fieldSize)
                data = bytes(ctype(VALUE)) * COUNT

                unsafe.set(srcPtr, data)
                self_.assertEqual(unsafe.get(srcPtr, fieldSize), data)

                func: Callable[[Ptr, Ptr, int], None] = getattr(simd, f"memcpy{name}Aligned")
                func(srcPtr, dstPtr, COUNT)

                self_.assertEqual(unsafe.get(dstPtr, fieldSize), data)

                unsafe.free(srcPtr)
                unsafe.free(dstPtr)

        setattr(TestSIMD, f"test_memCpy{name}", testMemcpy)
        setattr(TestSIMD, f"test_memCpy{name}Aligned", testMemcpyAligned)


initTestCases()

if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(TestSIMD)
    unittest.TextTestRunner().run(suite)
