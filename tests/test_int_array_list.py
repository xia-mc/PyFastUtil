import unittest
from pyfastutil.ints import IntArrayList


class MyTestCase(unittest.TestCase):
    def test_index(self):
        lst = IntArrayList([1, 2, 3])
        self.assertEqual(lst.index(1), 0)

    def test_len(self):
        lst = IntArrayList([1, 2, 3])
        self.assertEqual(len(lst), 3)

    def test_iter(self):
        lst = [1, 2, 3]
        newLst = []
        for i in IntArrayList(lst):
            newLst.append(i)
        self.assertEqual(lst, newLst)


if __name__ == '__main__':
    unittest.main()
