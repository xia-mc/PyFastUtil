import unittest
from pyfastutil.objects import ObjectArrayList
from tests.benchmark import benchmark_ObjectArrayList


class TestObjectArrayList(unittest.TestCase):

    # Test creation and basic properties
    def test_creation_empty(self):
        lst = ObjectArrayList()
        self.assertEqual(len(lst), 0)
        self.assertEqual(lst, [])

    def test_creation_with_values(self):
        lst = ObjectArrayList([1, 2, 3])
        self.assertEqual(len(lst), 3)
        self.assertEqual(lst, [1, 2, 3])

    # Test basic list operations
    def test_append(self):
        lst = ObjectArrayList([1, 2])
        lst.append(3)
        self.assertEqual(lst, [1, 2, 3])

    def test_insert(self):
        lst = ObjectArrayList([1, 3])
        lst.insert(1, 2)
        self.assertEqual(lst, [1, 2, 3])

    def test_extend(self):
        lst = ObjectArrayList([1, 2])
        lst.extend([3, 4])
        self.assertEqual(lst, [1, 2, 3, 4])

    def test_pop(self):
        lst = ObjectArrayList([1, 2, 3])
        popped = lst.pop()
        self.assertEqual(popped, 3)
        self.assertEqual(lst, [1, 2])

    def test_remove(self):
        lst = ObjectArrayList([1, 2, 3, 2])
        lst.remove(2)
        self.assertEqual(lst, [1, 3, 2])

    def test_clear(self):
        lst = ObjectArrayList([1, 2, 3])
        lst.clear()
        self.assertEqual(len(lst), 0)
        self.assertEqual(lst, [])

    # Test index and item access
    def test_getitem(self):
        lst = ObjectArrayList([1, 2, 3])
        self.assertEqual(lst[0], 1)
        self.assertEqual(lst[1], 2)
        self.assertEqual(lst[2], 3)

    def test_setitem(self):
        lst = ObjectArrayList([1, 2, 3])
        lst[1] = 99
        self.assertEqual(lst[1], 99)

    def test_delitem(self):
        lst = ObjectArrayList([1, 2, 3])
        del lst[1]
        self.assertEqual(lst, [1, 3])

    def test_slice(self):
        lst = ObjectArrayList([1, 2, 3, 4, 5])
        self.assertEqual(lst[:2], [1, 2])
        self.assertEqual(lst[2:], [3, 4, 5])
        self.assertEqual(lst[1:4], [2, 3, 4])

    def test_reverse(self):
        lst = ObjectArrayList([1, 2, 3])
        lst.reverse()
        self.assertEqual(lst, [3, 2, 1])

    def test_sort(self):
        lst = ObjectArrayList([3, 1, 2])
        lst.sort()
        self.assertEqual(lst, [1, 2, 3])

    def test_sort_reverse(self):
        lst = ObjectArrayList([3, 1, 2, 4, 7, 6, 5])
        lst.sort(reverse=True)
        self.assertEqual(lst, [7, 6, 5, 4, 3, 2, 1])

    def test_copy(self):
        lst = ObjectArrayList([1, 2, 3])
        lst_copy = lst.copy()
        self.assertEqual(lst, lst_copy)
        self.assertIsNot(lst, lst_copy)

    # Test list comparisons
    def test_equality(self):
        lst1 = ObjectArrayList([1, 2, 3])
        lst2 = ObjectArrayList([1, 2, 3])
        lst3 = ObjectArrayList([1, 2, 4])

        self.assertEqual(lst1, lst2)
        self.assertNotEqual(lst1, lst3)

    def test_contains(self):
        lst = ObjectArrayList([1, 2, 3])
        self.assertIn(2, lst)
        self.assertNotIn(4, lst)

    # Test list resizing
    def test_resize_increase(self):
        lst = ObjectArrayList([1, 2, 3])
        lst.resize(5)
        self.assertEqual(lst, [1, 2, 3, None, None])

    def test_resize_decrease(self):
        lst = ObjectArrayList([1, 2, 3])
        lst.resize(2)
        self.assertEqual(lst, [1, 2])

    # Test iteration
    def test_iter(self):
        lst = ObjectArrayList([1, 2, 3])
        iterated = [x for x in lst]
        self.assertEqual(iterated, [1, 2, 3])

    # Test count and index
    def test_count(self):
        lst = ObjectArrayList([1, 2, 2, 3])
        self.assertEqual(lst.count(2), 2)
        self.assertEqual(lst.count(1), 1)
        self.assertEqual(lst.count(4), 0)

    def test_index(self):
        lst = ObjectArrayList([1, 2, 3])
        self.assertEqual(lst.index(2), 1)
        with self.assertRaises(ValueError):
            lst.index(99)

    # Test exceptions
    def test_out_of_bounds(self):
        lst = ObjectArrayList([1, 2, 3])
        with self.assertRaises(IndexError):
            _ = lst[10]

        with self.assertRaises(IndexError):
            lst[10] = 99

    def test_invalid_resize(self):
        lst = ObjectArrayList([1, 2, 3])
        with self.assertRaises(ValueError):
            lst.resize(-1)

    def test_remove_nonexistent(self):
        lst = ObjectArrayList([1, 2, 3])
        with self.assertRaises(ValueError):
            lst.remove(99)

    def test_benchmark(self):
        self.assertEqual(benchmark_ObjectArrayList.main(), None)


if __name__ == '__main__':
    unittest.main()
