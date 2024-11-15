import unittest
from pyfastutil.ints import IntArrayList


class TestIntArrayList(unittest.TestCase):

    def test_basic_operations(self):
        # Test list creation and basic methods
        lst = IntArrayList([1, 2, 3, 4])
        self.assertEqual(len(lst), 4)
        self.assertEqual(lst[0], 1)
        self.assertEqual(lst[1], 2)
        self.assertEqual(lst[2], 3)
        self.assertEqual(lst[3], 4)

        # Test list slicing
        self.assertEqual(lst[:2], [1, 2])
        self.assertEqual(lst[1:3], [2, 3])
        self.assertEqual(lst[-1], 4)

        # Test list modification
        lst[1] = 20
        self.assertEqual(lst[1], 20)

        # Test append
        lst.append(5)
        self.assertEqual(lst[-1], 5)
        self.assertEqual(len(lst), 5)

        # Test insert
        lst.insert(2, 99)
        self.assertEqual(lst[2], 99)
        self.assertEqual(len(lst), 6)

        # Test pop
        popped = lst.pop()
        self.assertEqual(popped, 5)
        self.assertEqual(len(lst), 5)

        # Test remove
        lst.remove(99)
        self.assertNotIn(99, lst)
        self.assertEqual(len(lst), 4)

    def test_index(self):
        # Test index lookup
        lst = IntArrayList([10, 20, 30, 40])
        self.assertEqual(lst.index(10), 0)
        self.assertEqual(lst.index(30), 2)
        with self.assertRaises(ValueError):
            lst.index(100)

    def test_count(self):
        # Test count method
        lst = IntArrayList([1, 2, 2, 3, 4])
        self.assertEqual(lst.count(2), 2)
        self.assertEqual(lst.count(1), 1)
        self.assertEqual(lst.count(5), 0)

    def test_extend(self):
        # Test extend functionality
        lst = IntArrayList([1, 2, 3])
        lst.extend([4, 5, 6])
        self.assertEqual(lst, [1, 2, 3, 4, 5, 6])

    def test_reverse(self):
        # Test reverse method
        lst = IntArrayList([1, 2, 3, 4])
        lst.reverse()
        self.assertEqual(lst, [4, 3, 2, 1])

    def test_sort(self):
        # Test sort method
        lst = IntArrayList([4, 1, 3, 2])
        lst.sort()
        self.assertEqual(lst, [1, 2, 3, 4])

    def test_clear(self):
        # Test clear method
        lst = IntArrayList([1, 2, 3])
        lst.clear()
        self.assertEqual(len(lst), 0)

    def test_from_range(self):
        # Test from_range with start, stop, step
        lst = IntArrayList.from_range(1, 10, 2)
        self.assertEqual(lst, [1, 3, 5, 7, 9])

        # Test from_range with stop only
        lst = IntArrayList.from_range(5)
        self.assertEqual(lst, [0, 1, 2, 3, 4])

        # Test from_range with negative step
        lst = IntArrayList.from_range(10, 0, -2)
        self.assertEqual(lst, [10, 8, 6, 4, 2])

    def test_resize(self):
        # Test resizing to a larger size
        lst = IntArrayList([1, 2, 3])
        lst.resize(5)
        self.assertEqual(lst, [1, 2, 3, 0, 0])

        # Test resizing to a smaller size
        lst.resize(2)
        self.assertEqual(lst, [1, 2])

        # Test resizing to the same size
        lst.resize(2)
        self.assertEqual(lst, [1, 2])

    def test_copy(self):
        # Test copy method
        lst = IntArrayList([1, 2, 3])
        lst_copy = lst.copy()
        self.assertEqual(lst, lst_copy)
        self.assertIsNot(lst, lst_copy)  # Ensure it's a deep copy

    def test_equality(self):
        # Test equality and inequality
        lst1 = IntArrayList([1, 2, 3])
        lst2 = IntArrayList([1, 2, 3])
        lst3 = IntArrayList([1, 2, 4])

        self.assertEqual(lst1, lst2)
        self.assertNotEqual(lst1, lst3)

    def test_contains(self):
        # Test contains method
        lst = IntArrayList([1, 2, 3])
        self.assertIn(2, lst)
        self.assertNotIn(4, lst)

    def test_iter(self):
        # Test iteration
        lst = IntArrayList([1, 2, 3])
        iterated = [x for x in lst]
        self.assertEqual(iterated, [1, 2, 3])

    def test_exceptions(self):
        # Test out-of-bound access
        lst = IntArrayList([1, 2, 3])
        with self.assertRaises(IndexError):
            _ = lst[10]

        with self.assertRaises(IndexError):
            lst[10] = 99


if __name__ == '__main__':
    unittest.main()
