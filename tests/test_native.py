import unittest

from pyfastutil.native import native


@unittest.skipUnless(False, "@native is ")
class TestNative(unittest.TestCase):
    def test_add(self):
        def add(x, y):
            return x + y

        @native
        def nativeAdd(x, y):
            return x + y

        self.assertEqual(add(1, 1), nativeAdd(1, 1))
        self.assertEqual(add("1", "1"), nativeAdd("1", "1"))
        self.assertEqual(add(True, True), nativeAdd(True, True))
        self.assertEqual(add(1, True), nativeAdd(1, True))
        self.assertEqual(nativeAdd.__doc__, "<native method>")

    def test_binary_op(self):
        def op(x, y):
            return x + y * x * y

        @native
        def nativeOp(x, y):
            return x + y * x * y

        self.assertEqual(op(1, 1), nativeOp(1, 1))
        self.assertEqual(op(True, True), nativeOp(True, True))
        self.assertEqual(op(1, True), nativeOp(1, True))
        self.assertEqual(nativeOp.__doc__, "<native method>")

    def test_call(self):
        def call():
            return list(range(10))

        @native
        def nativeCall():
            return list(range(10))

        self.assertEqual(call(), nativeCall())
        self.assertEqual(nativeCall.__doc__, "<native method>")


if __name__ == '__main__':
    unittest.main()
