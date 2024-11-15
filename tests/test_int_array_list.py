import unittest
from pyfastutil.ints import IntArrayList
from tests.benchmark import benchmark_IntArrayList


class TestIntArrayList(unittest.TestCase):

    # Test creation and basic properties
    def test_creation_empty(self):
        lst = IntArrayList()
        self.assertEqual(len(lst), 0)
        self.assertEqual(lst, [])

    def test_creation_with_values(self):
        lst = IntArrayList([1, 2, 3])
        self.assertEqual(len(lst), 3)
        self.assertEqual(lst, [1, 2, 3])

    def test_creation_from_range(self):
        lst = IntArrayList.from_range(5)
        self.assertEqual(lst, [0, 1, 2, 3, 4])

        lst = IntArrayList.from_range(1, 10, 2)
        self.assertEqual(lst, [1, 3, 5, 7, 9])

        lst = IntArrayList.from_range(10, 0, -2)
        self.assertEqual(lst, [10, 8, 6, 4, 2])

    # Test basic list operations
    def test_append(self):
        lst = IntArrayList([1, 2])
        lst.append(3)
        self.assertEqual(lst, [1, 2, 3])

    def test_insert(self):
        lst = IntArrayList([1, 3])
        lst.insert(1, 2)
        self.assertEqual(lst, [1, 2, 3])

    def test_extend(self):
        lst = IntArrayList([1, 2])
        lst.extend([3, 4])
        self.assertEqual(lst, [1, 2, 3, 4])

    def test_pop(self):
        lst = IntArrayList([1, 2, 3])
        popped = lst.pop()
        self.assertEqual(popped, 3)
        self.assertEqual(lst, [1, 2])

    def test_remove(self):
        lst = IntArrayList([1, 2, 3, 2])
        lst.remove(2)
        self.assertEqual(lst, [1, 3, 2])

    def test_clear(self):
        lst = IntArrayList([1, 2, 3])
        lst.clear()
        self.assertEqual(len(lst), 0)
        self.assertEqual(lst, [])

    # Test index and item access
    def test_getitem(self):
        lst = IntArrayList([1, 2, 3])
        self.assertEqual(lst[0], 1)
        self.assertEqual(lst[1], 2)
        self.assertEqual(lst[2], 3)

    def test_setitem(self):
        lst = IntArrayList([1, 2, 3])
        lst[1] = 99
        self.assertEqual(lst[1], 99)

    def test_delitem(self):
        lst = IntArrayList([1, 2, 3])
        del lst[1]
        self.assertEqual(lst, [1, 3])

    def test_slice(self):
        lst = IntArrayList([1, 2, 3, 4, 5])
        self.assertEqual(lst[:2], [1, 2])
        self.assertEqual(lst[2:], [3, 4, 5])
        self.assertEqual(lst[1:4], [2, 3, 4])

    def test_reverse(self):
        lst = IntArrayList([1, 2, 3])
        lst.reverse()
        self.assertEqual(lst, [3, 2, 1])

    def test_sort(self):
        lst = IntArrayList([3, 1, 2])
        lst.sort()
        self.assertEqual(lst, [1, 2, 3])

    def test_copy(self):
        lst = IntArrayList([1, 2, 3])
        lst_copy = lst.copy()
        self.assertEqual(lst, lst_copy)
        self.assertIsNot(lst, lst_copy)

    # Test list comparisons
    def test_equality(self):
        lst1 = IntArrayList([1, 2, 3])
        lst2 = IntArrayList([1, 2, 3])
        lst3 = IntArrayList([1, 2, 4])

        self.assertEqual(lst1, lst2)
        self.assertNotEqual(lst1, lst3)

    def test_contains(self):
        lst = IntArrayList([1, 2, 3])
        self.assertIn(2, lst)
        self.assertNotIn(4, lst)

    # Test list resizing
    def test_resize_increase(self):
        lst = IntArrayList([1, 2, 3])
        lst.resize(5)
        self.assertEqual(lst, [1, 2, 3, 0, 0])

    def test_resize_decrease(self):
        lst = IntArrayList([1, 2, 3])
        lst.resize(2)
        self.assertEqual(lst, [1, 2])

    # Test iteration
    def test_iter(self):
        lst = IntArrayList([1, 2, 3])
        iterated = [x for x in lst]
        self.assertEqual(iterated, [1, 2, 3])

    # Test count and index
    def test_count(self):
        lst = IntArrayList([1, 2, 2, 3])
        self.assertEqual(lst.count(2), 2)
        self.assertEqual(lst.count(1), 1)
        self.assertEqual(lst.count(4), 0)

    def test_index(self):
        lst = IntArrayList([1, 2, 3])
        self.assertEqual(lst.index(2), 1)
        with self.assertRaises(ValueError):
            lst.index(99)

    # Test exceptions
    def test_out_of_bounds(self):
        lst = IntArrayList([1, 2, 3])
        with self.assertRaises(IndexError):
            _ = lst[10]

        with self.assertRaises(IndexError):
            lst[10] = 99

    def test_invalid_resize(self):
        lst = IntArrayList([1, 2, 3])
        with self.assertRaises(ValueError):
            lst.resize(-1)

    def test_remove_nonexistent(self):
        lst = IntArrayList([1, 2, 3])
        with self.assertRaises(ValueError):
            lst.remove(99)

    def test_benchmark(self):
        self.assertEqual(benchmark_IntArrayList.main(), None)


if __name__ == '__main__':
    unittest.main()
