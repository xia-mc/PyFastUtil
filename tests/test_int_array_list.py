import unittest
from pyfastutil.ints import IntArrayList


class MyTestCase(unittest.TestCase):
    def test_index(self):
        lst = IntArrayList([1, 2, 3])
        self.assertEqual(lst.index(3), 2)  # add assertion here


if __name__ == '__main__':
    unittest.main()
