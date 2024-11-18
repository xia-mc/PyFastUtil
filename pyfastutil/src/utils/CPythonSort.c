//
// Created by xia__mc on 2024/11/18.
// See https://github.com/python/cpython/blob/main/Objects/listobject.c
// The CPython impl for List sort
//

#include "CPythonSort.h"
#include "stddef.h"
#include "stdbool.h"

#ifdef Py_GIL_DISABLED
#define FT_ATOMIC_LOAD_PTR(value) _Py_atomic_load_ptr(&value)
#define FT_ATOMIC_STORE_PTR(value, new_value) _Py_atomic_store_ptr(&value, new_value)
#define FT_ATOMIC_LOAD_SSIZE(value) _Py_atomic_load_ssize(&value)
#define FT_ATOMIC_LOAD_SSIZE_ACQUIRE(value) \
    _Py_atomic_load_ssize_acquire(&value)
#define FT_ATOMIC_LOAD_SSIZE_RELAXED(value) \
    _Py_atomic_load_ssize_relaxed(&value)
#define FT_ATOMIC_STORE_PTR(value, new_value) \
    _Py_atomic_store_ptr(&value, new_value)
#define FT_ATOMIC_LOAD_PTR_ACQUIRE(value) \
    _Py_atomic_load_ptr_acquire(&value)
#define FT_ATOMIC_LOAD_UINTPTR_ACQUIRE(value) \
    _Py_atomic_load_uintptr_acquire(&value)
#define FT_ATOMIC_LOAD_PTR_RELAXED(value) \
    _Py_atomic_load_ptr_relaxed(&value)
#define FT_ATOMIC_LOAD_UINT8(value) \
    _Py_atomic_load_uint8(&value)
#define FT_ATOMIC_STORE_UINT8(value, new_value) \
    _Py_atomic_store_uint8(&value, new_value)
#define FT_ATOMIC_LOAD_UINT8_RELAXED(value) \
    _Py_atomic_load_uint8_relaxed(&value)
#define FT_ATOMIC_LOAD_UINT16_RELAXED(value) \
    _Py_atomic_load_uint16_relaxed(&value)
#define FT_ATOMIC_LOAD_UINT32_RELAXED(value) \
    _Py_atomic_load_uint32_relaxed(&value)
#define FT_ATOMIC_LOAD_ULONG_RELAXED(value) \
    _Py_atomic_load_ulong_relaxed(&value)
#define FT_ATOMIC_STORE_PTR_RELAXED(value, new_value) \
    _Py_atomic_store_ptr_relaxed(&value, new_value)
#define FT_ATOMIC_STORE_PTR_RELEASE(value, new_value) \
    _Py_atomic_store_ptr_release(&value, new_value)
#define FT_ATOMIC_STORE_UINTPTR_RELEASE(value, new_value) \
    _Py_atomic_store_uintptr_release(&value, new_value)
#define FT_ATOMIC_STORE_SSIZE_RELAXED(value, new_value) \
    _Py_atomic_store_ssize_relaxed(&value, new_value)
#define FT_ATOMIC_STORE_UINT8_RELAXED(value, new_value) \
    _Py_atomic_store_uint8_relaxed(&value, new_value)
#define FT_ATOMIC_STORE_UINT16_RELAXED(value, new_value) \
    _Py_atomic_store_uint16_relaxed(&value, new_value)
#define FT_ATOMIC_STORE_UINT32_RELAXED(value, new_value) \
    _Py_atomic_store_uint32_relaxed(&value, new_value)

#else
#define FT_ATOMIC_LOAD_PTR(value) value
#define FT_ATOMIC_STORE_PTR(value, new_value) value = new_value
#define FT_ATOMIC_LOAD_SSIZE(value) value
#define FT_ATOMIC_LOAD_SSIZE_ACQUIRE(value) value
#define FT_ATOMIC_LOAD_SSIZE_RELAXED(value) value
#define FT_ATOMIC_STORE_PTR(value, new_value) value = new_value
#define FT_ATOMIC_LOAD_PTR_ACQUIRE(value) value
#define FT_ATOMIC_LOAD_UINTPTR_ACQUIRE(value) value
#define FT_ATOMIC_LOAD_PTR_RELAXED(value) value
#define FT_ATOMIC_LOAD_UINT8(value) value
#define FT_ATOMIC_STORE_UINT8(value, new_value) value = new_value
#define FT_ATOMIC_LOAD_UINT8_RELAXED(value) value
#define FT_ATOMIC_LOAD_UINT16_RELAXED(value) value
#define FT_ATOMIC_LOAD_UINT32_RELAXED(value) value
#define FT_ATOMIC_LOAD_ULONG_RELAXED(value) value
#define FT_ATOMIC_STORE_PTR_RELAXED(value, new_value) value = new_value
#define FT_ATOMIC_STORE_PTR_RELEASE(value, new_value) value = new_value
#define FT_ATOMIC_STORE_UINTPTR_RELEASE(value, new_value) value = new_value
#define FT_ATOMIC_STORE_SSIZE_RELAXED(value, new_value) value = new_value
#define FT_ATOMIC_STORE_UINT8_RELAXED(value, new_value) value = new_value
#define FT_ATOMIC_STORE_UINT16_RELAXED(value, new_value) value = new_value
#define FT_ATOMIC_STORE_UINT32_RELAXED(value, new_value) value = new_value

#endif

/* The maximum number of entries in a MergeState's pending-runs stack.
 * For a list with n elements, this needs at most floor(log2(n)) + 1 entries
 * even if we didn't force runs to a minimal length.  So the number of bits
 * in a Py_ssize_t is plenty large enough for all cases.
 */
#define MAX_MERGE_PENDING (SIZEOF_SIZE_T * 8)

/* When we get into galloping mode, we stay there until both runs win less
 * often than MIN_GALLOP consecutive times.  See listsort.txt for more info.
 */
#define MIN_GALLOP 7

/* Avoid malloc for small temp arrays. */
#define MERGESTATE_TEMP_SIZE 256

/* The largest value of minrun. This must be a power of 2, and >= 1, so that
 * the compute_minrun() algorithm guarantees to return a result no larger than
 * this,
 */
#define MAX_MINRUN 64
#if ((MAX_MINRUN) < 1) || ((MAX_MINRUN) & ((MAX_MINRUN) - 1))
#error "MAX_MINRUN must be a power of 2, and >= 1"
#endif

#define ISLT(X, Y) (*(ms->key_compare))(X, Y, ms)

/* Compare X to Y via "<".  Goto "fail" if the comparison raises an
   error.  Else "k" is set to true iff X<Y, and an "if (k)" block is
   started.  It makes more sense in context <wink>.  X and Y are PyObject*s.
*/
#define IFLT(X, Y) if ((k = ISLT(X, Y)) < 0) goto fail;  \
           if (k)

static void
free_list_items(PyObject **items, bool use_qsbr) {
#ifdef Py_GIL_DISABLED
    _PyListArray *array = _Py_CONTAINER_OF(items, _PyListArray, ob_item);
    if (use_qsbr) {
        _PyMem_FreeDelayed(array);
    }
    else {
        PyMem_Free(array);
    }
#else
    (void)use_qsbr;
    PyMem_Free(items);
#endif
}

/* Reverse a slice of a list in place, from lo up to (exclusive) hi. */
static void
reverse_slice(PyObject **lo, PyObject **hi) {
    assert(lo && hi);

    --hi;
    while (lo < hi) {
        PyObject *t = *lo;
        *lo = *hi;
        *hi = t;
        ++lo;
        --hi;
    }
}


/* A sortslice contains a pointer to an array of keys and a pointer to
 * an array of corresponding values.  In other words, keys[i]
 * corresponds with values[i].  If values == NULL, then the keys are
 * also the values.
 *
 * Several convenience routines are provided here, so that keys and
 * values are always moved in sync.
 */

typedef struct {
    PyObject **keys;
    PyObject **values;
} sortslice;

/* One MergeState exists on the stack per invocation of mergesort.  It's just
 * a convenient way to pass state around among the helper functions.
 */
struct s_slice {
    sortslice base;
    Py_ssize_t len;   /* length of run */
    int power; /* node "level" for powersort merge strategy */
};

typedef struct s_MergeState MergeState;

struct s_MergeState {
    /* This controls when we get *into* galloping mode.  It's initialized
     * to MIN_GALLOP.  merge_lo and merge_hi tend to nudge it higher for
     * random data, and lower for highly structured data.
     */
    Py_ssize_t min_gallop;

    Py_ssize_t listlen;     /* len(input_list) - read only */
    PyObject **basekeys;    /* base address of keys array - read only */

    /* 'a' is temp storage to help with merges.  It contains room for
     * alloced entries.
     */
    sortslice a;        /* may point to temparray below */
    Py_ssize_t alloced;

    /* A stack of n pending runs yet to be merged.  Run #i starts at
     * address base[i] and extends for len[i] elements.  It's always
     * true (so long as the indices are in bounds) that
     *
     *     pending[i].base + pending[i].len == pending[i+1].base
     *
     * so we could cut the storage for this, but it's a minor amount,
     * and keeping all the info explicit simplifies the code.
     */
    int n;
    struct s_slice pending[MAX_MERGE_PENDING];

    /* 'a' points to this when possible, rather than muck with malloc. */
    PyObject *temparray[MERGESTATE_TEMP_SIZE];

    /* This is the function we will use to compare two keys,
     * even when none of our special cases apply and we have to use
     * safe_object_compare. */
    int (*key_compare)(PyObject *, PyObject *, MergeState *);

    /* This function is used by unsafe_object_compare to optimize comparisons
     * when we know our list is type-homogeneous but we can't assume anything else.
     * In the pre-sort check it is set equal to Py_TYPE(key)->tp_richcompare */
    PyObject *(*key_richcompare)(PyObject *, PyObject *, int);

    /* This function is used by unsafe_tuple_compare to compare the first elements
     * of tuples. It may be set to safe_object_compare, but the idea is that hopefully
     * we can assume more, and use one of the special-case compares. */
    int (*tuple_elem_compare)(PyObject *, PyObject *, MergeState *);
};

/*
Exactly like gallop_left(), except that if key already exists in a[0:n],
finds the position immediately to the right of the rightmost equal value.

The return value is the int k in 0..n such that

    a[k-1] <= key < a[k]

or -1 if error.

The code duplication is massive, but this is enough different given that
we're sticking to "<" comparisons that it's much harder to follow if
written as one routine with yet another "left or right?" flag.
*/
static __forceinline Py_ssize_t
gallop_right(MergeState *ms, PyObject *key, PyObject **a, Py_ssize_t n, Py_ssize_t hint) {
    Py_ssize_t ofs;
    Py_ssize_t lastofs;
    Py_ssize_t k;

    assert(key && a && n > 0 && hint >= 0 && hint < n);

    a += hint;
    lastofs = 0;
    ofs = 1;
    IFLT(key, *a) {
        /* key < a[hint] -- gallop left, until
         * a[hint - ofs] <= key < a[hint - lastofs]
         */
        const Py_ssize_t maxofs = hint + 1;             /* &a[0] is lowest */
        while (ofs < maxofs) {
            IFLT(key, *(a - ofs)) {
                lastofs = ofs;
                assert(ofs <= (PY_SSIZE_T_MAX - 1) / 2);
                ofs = (ofs << 1) + 1;
            } else                /* a[hint - ofs] <= key */
                break;
        }
        if (ofs > maxofs)
            ofs = maxofs;
        /* Translate back to positive offsets relative to &a[0]. */
        k = lastofs;
        lastofs = hint - ofs;
        ofs = hint - k;
    } else {
        /* a[hint] <= key -- gallop right, until
         * a[hint + lastofs] <= key < a[hint + ofs]
        */
        const Py_ssize_t maxofs = n - hint;             /* &a[n-1] is highest */
        while (ofs < maxofs) {
            IFLT(key, a[ofs])break;
            /* a[hint + ofs] <= key */
            lastofs = ofs;
            assert(ofs <= (PY_SSIZE_T_MAX - 1) / 2);
            ofs = (ofs << 1) + 1;
        }
        if (ofs > maxofs)
            ofs = maxofs;
        /* Translate back to offsets relative to &a[0]. */
        lastofs += hint;
        ofs += hint;
    }
    a -= hint;

    assert(-1 <= lastofs && lastofs < ofs && ofs <= n);
    /* Now a[lastofs] <= key < a[ofs], so key belongs somewhere to the
     * right of lastofs but no farther right than ofs.  Do a binary
     * search, with invariant a[lastofs-1] <= key < a[ofs].
     */
    ++lastofs;
    while (lastofs < ofs) {
        Py_ssize_t m = lastofs + ((ofs - lastofs) >> 1);

        IFLT(key, a[m])ofs = m;                    /* key < a[m] */
        else
            lastofs = m + 1;              /* a[m] <= key */
    }
    assert(lastofs == ofs);             /* so a[ofs-1] <= key < a[ofs] */
    return ofs;

    fail:
    return -1;
}

/* Conceptually a MergeState's constructor. */
static void
merge_init(MergeState *ms, Py_ssize_t list_size, int has_keyfunc,
           sortslice *lo) {
    assert(ms != NULL);
    if (has_keyfunc) {
        /* The temporary space for merging will need at most half the list
         * size rounded up.  Use the minimum possible space so we can use the
         * rest of temparray for other things.  In particular, if there is
         * enough extra space, listsort() will use it to store the keys.
         */
        ms->alloced = (list_size + 1) / 2;

        /* ms->alloced describes how many keys will be stored at
           ms->temparray, but we also need to store the values.  Hence,
           ms->alloced is capped at half of MERGESTATE_TEMP_SIZE. */
        if (MERGESTATE_TEMP_SIZE / 2 < ms->alloced)
            ms->alloced = MERGESTATE_TEMP_SIZE / 2;
        ms->a.values = &ms->temparray[ms->alloced];
    } else {
        ms->alloced = MERGESTATE_TEMP_SIZE;
        ms->a.values = NULL;
    }
    ms->a.keys = ms->temparray;
    ms->n = 0;
    ms->min_gallop = MIN_GALLOP;
    ms->listlen = list_size;
    ms->basekeys = lo->keys;
}

/* Compute a good value for the minimum run length; natural runs shorter
 * than this are boosted artificially via binary insertion.
 *
 * If n < MAX_MINRUN return n (it's too small to bother with fancy stuff).
 * Else if n is an exact power of 2, return MAX_MINRUN / 2.
 * Else return an int k, MAX_MINRUN / 2 <= k <= MAX_MINRUN, such that n/k is
 * close to, but strictly less than, an exact power of 2.
 *
 * See listsort.txt for more info.
 */
static __forceinline Py_ssize_t
merge_compute_minrun(Py_ssize_t n) {
    Py_ssize_t r = 0;           /* becomes 1 if any 1 bits are shifted off */

    assert(n >= 0);
    while (n >= MAX_MINRUN) {
        r |= n & 1;
        n >>= 1;
    }
    return n + r;
}

/* Two adjacent runs begin at index s1. The first run has length n1, and
 * the second run (starting at index s1+n1) has length n2. The list has total
 * length n.
 * Compute the "power" of the first run. See listsort.txt for details.
 */
static int
powerloop(Py_ssize_t s1, Py_ssize_t n1, Py_ssize_t n2, Py_ssize_t n) {
    int result = 0;
    assert(s1 >= 0);
    assert(n1 > 0 && n2 > 0);
    assert(s1 + n1 + n2 <= n);
    /* midpoints a and b:
     * a = s1 + n1/2
     * b = s1 + n1 + n2/2 = a + (n1 + n2)/2
     *
     * Those may not be integers, though, because of the "/2". So we work with
     * 2*a and 2*b instead, which are necessarily integers. It makes no
     * difference to the outcome, since the bits in the expansion of (2*i)/n
     * are merely shifted one position from those of i/n.
     */
    Py_ssize_t a = 2 * s1 + n1;  /* 2*a */
    Py_ssize_t b = a + n1 + n2;  /* 2*b */
    /* Emulate a/n and b/n one bit a time, until bits differ. */
    for (;;) {
        ++result;
        if (a >= n) {  /* both quotient bits are 1 */
            assert(b >= a);
            a -= n;
            b -= n;
        } else if (b >= n) {  /* a/n bit is 0, b/n bit is 1 */
            break;
        } /* else both quotient bits are 0 */
        assert(a < b && b < n);
        a <<= 1;
        b <<= 1;
    }
    return result;
}

static __forceinline void
sortslice_memcpy(sortslice *s1, Py_ssize_t i, sortslice *s2, Py_ssize_t j,
                 Py_ssize_t n) {
    memcpy(&s1->keys[i], &s2->keys[j], sizeof(PyObject *) * n);
    if (s1->values != NULL)
        memcpy(&s1->values[i], &s2->values[j], sizeof(PyObject *) * n);
}

static __forceinline void
sortslice_memmove(sortslice *s1, Py_ssize_t i, sortslice *s2, Py_ssize_t j,
                  Py_ssize_t n) {
    memmove(&s1->keys[i], &s2->keys[j], sizeof(PyObject *) * n);
    if (s1->values != NULL)
        memmove(&s1->values[i], &s2->values[j], sizeof(PyObject *) * n);
}

static __forceinline void
sortslice_advance(sortslice *slice, Py_ssize_t n) {
    slice->keys += n;
    if (slice->values != NULL)
        slice->values += n;
}

static __forceinline void
sortslice_copy(sortslice *s1, Py_ssize_t i, sortslice *s2, Py_ssize_t j) {
    s1->keys[i] = s2->keys[j];
    if (s1->values != NULL)
        s1->values[i] = s2->values[j];
}


static __forceinline void
sortslice_copy_incr(sortslice *dst, sortslice *src) {
    *dst->keys++ = *src->keys++;
    if (dst->values != NULL)
        *dst->values++ = *src->values++;
}

static __forceinline void
sortslice_copy_decr(sortslice *dst, sortslice *src) {
    *dst->keys-- = *src->keys--;
    if (dst->values != NULL)
        *dst->values-- = *src->values--;
}

static __forceinline void
sortslice_reverse(sortslice *s, Py_ssize_t n) {
    reverse_slice(s->keys, &s->keys[n]);
    if (s->values != NULL)
        reverse_slice(s->values, &s->values[n]);
}

/*
Return the length of the run beginning at slo->keys, spanning no more than
nremaining elements. The run beginning there may be ascending or descending,
but the function permutes it in place, if needed, so that it's always ascending
upon return.

Returns -1 in case of error.
*/
static Py_ssize_t
count_run(MergeState *ms, sortslice *slo, Py_ssize_t nremaining) {
    Py_ssize_t k; /* used by IFLT macro expansion */
    Py_ssize_t n;
    PyObject **const lo = slo->keys;

    /* In general, as things go on we've established that the slice starts
       with a monotone run of n elements, starting at lo. */

    /* We're n elements into the slice, and the most recent neq+1 elements are
     * all equal. This reverses them in-place, and resets neq for reuse.
     */
#define REVERSE_LAST_NEQ                        \
    if (neq) {                                  \
        sortslice slice = *slo;                 \
        ++neq;                                  \
        sortslice_advance(&slice, n - neq);     \
        sortslice_reverse(&slice, neq);         \
        neq = 0;                                \
    }

    /* Sticking to only __lt__ compares is confusing and error-prone. But in
     * this routine, almost all uses of IFLT can be captured by tiny macros
     * giving mnemonic names to the intent. Note that inline functions don't
     * work for this (IFLT expands to code including `goto fail`).
     */
#define IF_NEXT_LARGER  IFLT(lo[n-1], lo[n])
#define IF_NEXT_SMALLER IFLT(lo[n], lo[n-1])

    assert(nremaining);
    /* try ascending run first */
    for (n = 1; n < nremaining; ++n) {
        IF_NEXT_SMALLER break;
    }
    if (n == nremaining)
        return n;
    /* lo[n] is strictly less */
    /* If n is 1 now, then the first compare established it's a descending
     * run, so fall through to the descending case. But if n > 1, there are
     * n elements in an ascending run terminated by the strictly less lo[n].
     * If the first key < lo[n-1], *somewhere* along the way the sequence
     * increased, so we're done (there is no descending run).
     * Else first key >= lo[n-1], which implies that the entire ascending run
     * consists of equal elements. In that case, this is a descending run,
     * and we reverse the all-equal prefix in-place.
     */
    if (n > 1) {
        IFLT(lo[0], lo[n - 1])return n;
        sortslice_reverse(slo, n);
    }
    ++n; /* in all cases it's been established that lo[n] has been resolved */

    /* Finish descending run. All-squal subruns are reversed in-place on the
     * fly. Their original order will be restored at the end by the whole-slice
     * reversal.
     */
    Py_ssize_t neq = 0;
    for (; n < nremaining; ++n) {
        IF_NEXT_SMALLER {
            /* This ends the most recent run of equal elements, but still in
             * the "descending" direction.
             */
            REVERSE_LAST_NEQ
        } else {
            IF_NEXT_LARGER /* descending run is over */
                break;
            else /* not x < y and not y < x implies x == y */
                ++neq;
        }
    }
    REVERSE_LAST_NEQ
    sortslice_reverse(slo, n); /* transform to ascending run */

    /* And after reversing, it's possible this can be extended by a
     * naturally increasing suffix; e.g., [3, 2, 3, 4, 1] makes an
     * ascending run from the first 4 elements.
     */
    for (; n < nremaining; ++n) {
        IF_NEXT_SMALLER break;
    }

    return n;
    fail:
    return -1;

#undef REVERSE_LAST_NEQ
#undef IF_NEXT_SMALLER
#undef IF_NEXT_LARGER
}

/* binarysort is the best method for sorting small arrays: it does few
   compares, but can do data movement quadratic in the number of elements.
   ss->keys is viewed as an array of n kays, a[:n]. a[:ok] is already sorted.
   Pass ok = 0 (or 1) if you don't know.
   It's sorted in-place, by a stable binary insertion sort. If ss->values
   isn't NULL, it's permuted in lockstap with ss->keys.
   On entry, must have n >= 1, and 0 <= ok <= n <= MAX_MINRUN.
   Return -1 if comparison raises an exception, else 0.
   Even in case of error, the output slice will be some permutation of
   the input (nothing is lost or duplicated).
*/
static int
binarysort(MergeState *ms, const sortslice *ss, Py_ssize_t n, Py_ssize_t ok) {
    Py_ssize_t k; /* for IFLT macro expansion */
    PyObject **const a = ss->keys;
    PyObject **const v = ss->values;
    const bool has_values = v != NULL;
    PyObject *pivot;
    Py_ssize_t M;

    assert(0 <= ok && ok <= n && 1 <= n && n <= MAX_MINRUN);
    /* assert a[:ok] is sorted */
    if (!ok)
        ++ok;
    /* Regular insertion sort has average- and worst-case O(n**2) cost
       for both # of comparisons and number of bytes moved. But its branches
       are highly predictable, and it loves sorted input (n-1 compares and no
       data movement). This is significant in cases like sortperf.py's %sort,
       where an out-of-order element near the start of a run is moved into
       place slowly but then the remaining elements up to length minrun are
       generally at worst one slot away from their correct position (so only
       need 1 or 2 commpares to resolve). If comparisons are very fast (such
       as for a list of Python floats), the simple inner loop leaves it
       very competitive with binary insertion, despite that it does
       significantly more compares overall on random data.

       Binary insertion sort has worst, average, and best case O(n log n)
       cost for # of comparisons, but worst and average case O(n**2) cost
       for data movement. The more expensive comparisons, the more important
       the comparison advantage. But its branches are less predictable the
       more "randomish" the data, and that's so significant its worst case
       in real life is random input rather than reverse-ordered (which does
       about twice the data movement than random input does).

       Note that the number of bytes moved doesn't seem to matter. MAX_MINRUN
       of 64 is so small that the key and value pointers all fit in a corner
       of L1 cache, and moving things around in that is very fast. */
#if 0 // ordinary insertion sort.
    PyObject * vpivot = NULL;
    for (; ok < n; ++ok) {
        pivot = a[ok];
        if (has_values)
            vpivot = v[ok];
        for (M = ok - 1; M >= 0; --M) {
            k = ISLT(pivot, a[M]);
            if (k < 0) {
                a[M + 1] = pivot;
                if (has_values)
                    v[M + 1] = vpivot;
                goto fail;
            }
            else if (k) {
                a[M + 1] = a[M];
                if (has_values)
                    v[M + 1] = v[M];
            }
            else
                break;
        }
        a[M + 1] = pivot;
        if (has_values)
            v[M + 1] = vpivot;
    }
#else // binary insertion sort
    Py_ssize_t L, R;
    for (; ok < n; ++ok) {
        /* set L to where a[ok] belongs */
        L = 0;
        R = ok;
        pivot = a[ok];
        /* Slice invariants. vacuously true at the start:
         * all a[0:L]  <= pivot
         * all a[L:R]     unknown
         * all a[R:ok]  > pivot
         */
        assert(L < R);
        do {
            /* don't do silly ;-) things to prevent overflow when finding
               the midpoint; L and R are very far from filling a Py_ssize_t */
            M = (L + R) >> 1;
#if 1 // straightforward, but highly unpredictable branch on random data
            IFLT(pivot, a[M])R = M;
            else
                L = M + 1;
#else
            /* Try to get compiler to generate conditional move instructions
               instead. Works fine, but leaving it disabled for now because
               it's not yielding consistently faster sorts. Needs more
               investigation. More computation in the inner loop adds its own
               costs, which can be significant when compares are fast. */
            k = ISLT(pivot, a[M]);
            if (k < 0)
                goto fail;
            Py_ssize_t Mp1 = M + 1;
            R = k ? M : R;
            L = k ? L : Mp1;
#endif
        } while (L < R);
        assert(L == R);
        /* a[:L] holds all elements from a[:ok] <= pivot now, so pivot belongs
           at index L. Slide a[L:ok] to the right a slot to make room for it.
           Caution: using memmove is much slower under MSVC 5; we're not
           usually moving many slots. Years later: under Visual Studio 2022,
           memmove seems just slightly slower than doing it "by hand". */
        for (M = ok; M > L; --M)
            a[M] = a[M - 1];
        a[L] = pivot;
        if (has_values) {
            pivot = v[ok];
            for (M = ok; M > L; --M)
                v[M] = v[M - 1];
            v[L] = pivot;
        }
}
#endif // pick binary or regular insertion sort
return 0;

fail:
return -1;
}

/* Free all the temp memory owned by the MergeState.  This must be called
 * when you're done with a MergeState, and may be called before then if
 * you want to free the temp memory early.
 */
static __forceinline void
merge_freemem(MergeState *ms) {
    assert(ms != NULL);
    if (ms->a.keys != ms->temparray) {
        PyMem_Free(ms->a.keys);
        ms->a.keys = NULL;
    }
}

/*
Locate the proper position of key in a sorted vector; if the vector contains
an element equal to key, return the position immediately to the left of
the leftmost equal element.  [gallop_right() does the same except returns
the position to the right of the rightmost equal element (if any).]

"a" is a sorted vector with n elements, starting at a[0].  n must be > 0.

"hint" is an index at which to begin the search, 0 <= hint < n.  The closer
hint is to the final result, the faster this runs.

The return value is the int k in 0..n such that

    a[k-1] < key <= a[k]

pretending that *(a-1) is minus infinity and a[n] is plus infinity.  IOW,
key belongs at index k; or, IOW, the first k elements of a should precede
key, and the last n-k should follow key.

Returns -1 on error.  See listsort.txt for info on the method.
*/
static Py_ssize_t
gallop_left(MergeState *ms, PyObject *key, PyObject **a, Py_ssize_t n, Py_ssize_t hint) {
    Py_ssize_t ofs;
    Py_ssize_t lastofs;
    Py_ssize_t k;

    assert(key && a && n > 0 && hint >= 0 && hint < n);

    a += hint;
    lastofs = 0;
    ofs = 1;
    IFLT(*a, key) {
        /* a[hint] < key -- gallop right, until
         * a[hint + lastofs] < key <= a[hint + ofs]
         */
        const Py_ssize_t maxofs = n - hint;             /* &a[n-1] is highest */
        while (ofs < maxofs) {
            IFLT(a[ofs], key) {
                lastofs = ofs;
                assert(ofs <= (PY_SSIZE_T_MAX - 1) / 2);
                ofs = (ofs << 1) + 1;
            } else                /* key <= a[hint + ofs] */
                break;
        }
        if (ofs > maxofs)
            ofs = maxofs;
        /* Translate back to offsets relative to &a[0]. */
        lastofs += hint;
        ofs += hint;
    } else {
        /* key <= a[hint] -- gallop left, until
         * a[hint - ofs] < key <= a[hint - lastofs]
         */
        const Py_ssize_t maxofs = hint + 1;             /* &a[0] is lowest */
        while (ofs < maxofs) {
            IFLT(*(a - ofs), key)break;
            /* key <= a[hint - ofs] */
            lastofs = ofs;
            assert(ofs <= (PY_SSIZE_T_MAX - 1) / 2);
            ofs = (ofs << 1) + 1;
        }
        if (ofs > maxofs)
            ofs = maxofs;
        /* Translate back to positive offsets relative to &a[0]. */
        k = lastofs;
        lastofs = hint - ofs;
        ofs = hint - k;
    }
    a -= hint;

    assert(-1 <= lastofs && lastofs < ofs && ofs <= n);
    /* Now a[lastofs] < key <= a[ofs], so key belongs somewhere to the
     * right of lastofs but no farther right than ofs.  Do a binary
     * search, with invariant a[lastofs-1] < key <= a[ofs].
     */
    ++lastofs;
    while (lastofs < ofs) {
        Py_ssize_t m = lastofs + ((ofs - lastofs) >> 1);

        IFLT(a[m], key)lastofs = m + 1;              /* a[m] < key */
        else
            ofs = m;                    /* key <= a[m] */
    }
    assert(lastofs == ofs);             /* so a[ofs-1] < key <= a[ofs] */
    return ofs;

    fail:
    return -1;
}

/* Ensure enough temp memory for 'need' array slots is available.
 * Returns 0 on success and -1 if the memory can't be gotten.
 */
static __forceinline int
merge_getmem(MergeState *ms, Py_ssize_t need) {
    int multiplier;

    assert(ms != NULL);
    if (need <= ms->alloced)
        return 0;

    multiplier = ms->a.values != NULL ? 2 : 1;

    /* Don't realloc!  That can cost cycles to copy the old data, but
     * we don't care what's in the block.
     */
    merge_freemem(ms);
    if ((size_t) need > PY_SSIZE_T_MAX / sizeof(PyObject *) / multiplier) {
        PyErr_NoMemory();
        return -1;
    }
    ms->a.keys = (PyObject **) PyMem_Malloc(multiplier * need
                                            * sizeof(PyObject *));
    if (ms->a.keys != NULL) {
        ms->alloced = need;
        if (ms->a.values != NULL)
            ms->a.values = &ms->a.keys[need];
        return 0;
    }
    PyErr_NoMemory();
    return -1;
}

#define MERGE_GETMEM(MS, NEED) ((NEED) <= (MS)->alloced ? 0 :   \
                                merge_getmem(MS, NEED))

/* Merge the na elements starting at ssa with the nb elements starting at
 * ssb.keys = ssa.keys + na in a stable way, in-place.  na and nb must be > 0.
 * Must also have that ssa.keys[na-1] belongs at the end of the merge, and
 * should have na <= nb.  See listsort.txt for more info.  Return 0 if
 * successful, -1 if error.
 */
static Py_ssize_t
merge_lo(MergeState *ms, sortslice ssa, Py_ssize_t na,
         sortslice ssb, Py_ssize_t nb) {
    Py_ssize_t k;
    sortslice dest;
    int result = -1;            /* guilty until proved innocent */
    Py_ssize_t min_gallop;

    assert(ms && ssa.keys && ssb.keys && na > 0 && nb > 0);
    assert(ssa.keys + na == ssb.keys);
    if (MERGE_GETMEM(ms, na) < 0)
        return -1;
    sortslice_memcpy(&ms->a, 0, &ssa, 0, na);
    dest = ssa;
    ssa = ms->a;

    sortslice_copy_incr(&dest, &ssb);
    --nb;
    if (nb == 0)
        goto Succeed;
    if (na == 1)
        goto CopyB;

    min_gallop = ms->min_gallop;
    for (;;) {
        Py_ssize_t acount = 0;          /* # of times A won in a row */
        Py_ssize_t bcount = 0;          /* # of times B won in a row */

        /* Do the straightforward thing until (if ever) one run
         * appears to win consistently.
         */
        for (;;) {
            assert(na > 1 && nb > 0);
            k = ISLT(ssb.keys[0], ssa.keys[0]);
            if (k) {
                if (k < 0)
                    goto Fail;
                sortslice_copy_incr(&dest, &ssb);
                ++bcount;
                acount = 0;
                --nb;
                if (nb == 0)
                    goto Succeed;
                if (bcount >= min_gallop)
                    break;
            } else {
                sortslice_copy_incr(&dest, &ssa);
                ++acount;
                bcount = 0;
                --na;
                if (na == 1)
                    goto CopyB;
                if (acount >= min_gallop)
                    break;
            }
        }

        /* One run is winning so consistently that galloping may
         * be a huge win.  So try that, and continue galloping until
         * (if ever) neither run appears to be winning consistently
         * anymore.
         */
        ++min_gallop;
        do {
            assert(na > 1 && nb > 0);
            min_gallop -= min_gallop > 1;
            ms->min_gallop = min_gallop;
            k = gallop_right(ms, ssb.keys[0], ssa.keys, na, 0);
            acount = k;
            if (k) {
                if (k < 0)
                    goto Fail;
                sortslice_memcpy(&dest, 0, &ssa, 0, k);
                sortslice_advance(&dest, k);
                sortslice_advance(&ssa, k);
                na -= k;
                if (na == 1)
                    goto CopyB;
                /* na==0 is impossible now if the comparison
                 * function is consistent, but we can't assume
                 * that it is.
                 */
                if (na == 0)
                    goto Succeed;
            }
            sortslice_copy_incr(&dest, &ssb);
            --nb;
            if (nb == 0)
                goto Succeed;

            k = gallop_left(ms, ssa.keys[0], ssb.keys, nb, 0);
            bcount = k;
            if (k) {
                if (k < 0)
                    goto Fail;
                sortslice_memmove(&dest, 0, &ssb, 0, k);
                sortslice_advance(&dest, k);
                sortslice_advance(&ssb, k);
                nb -= k;
                if (nb == 0)
                    goto Succeed;
            }
            sortslice_copy_incr(&dest, &ssa);
            --na;
            if (na == 1)
                goto CopyB;
        } while (acount >= MIN_GALLOP || bcount >= MIN_GALLOP);
        ++min_gallop;           /* penalize it for leaving galloping mode */
        ms->min_gallop = min_gallop;
    }
    Succeed:
    result = 0;
    Fail:
    if (na)
        sortslice_memcpy(&dest, 0, &ssa, 0, na);
    return result;
    CopyB:
    assert(na == 1 && nb > 0);
    /* The last element of ssa belongs at the end of the merge. */
    sortslice_memmove(&dest, 0, &ssb, 0, nb);
    sortslice_copy(&dest, nb, &ssa, 0);
    return 0;
}

/* Merge the na elements starting at pa with the nb elements starting at
 * ssb.keys = ssa.keys + na in a stable way, in-place.  na and nb must be > 0.
 * Must also have that ssa.keys[na-1] belongs at the end of the merge, and
 * should have na >= nb.  See listsort.txt for more info.  Return 0 if
 * successful, -1 if error.
 */
static Py_ssize_t
merge_hi(MergeState *ms, sortslice ssa, Py_ssize_t na,
         sortslice ssb, Py_ssize_t nb) {
    Py_ssize_t k;
    sortslice dest, basea, baseb;
    int result = -1;            /* guilty until proved innocent */
    Py_ssize_t min_gallop;

    assert(ms && ssa.keys && ssb.keys && na > 0 && nb > 0);
    assert(ssa.keys + na == ssb.keys);
    if (MERGE_GETMEM(ms, nb) < 0)
        return -1;
    dest = ssb;
    sortslice_advance(&dest, nb - 1);
    sortslice_memcpy(&ms->a, 0, &ssb, 0, nb);
    basea = ssa;
    baseb = ms->a;
    ssb.keys = ms->a.keys + nb - 1;
    if (ssb.values != NULL)
        ssb.values = ms->a.values + nb - 1;
    sortslice_advance(&ssa, na - 1);

    sortslice_copy_decr(&dest, &ssa);
    --na;
    if (na == 0)
        goto Succeed;
    if (nb == 1)
        goto CopyA;

    min_gallop = ms->min_gallop;
    for (;;) {
        Py_ssize_t acount = 0;          /* # of times A won in a row */
        Py_ssize_t bcount = 0;          /* # of times B won in a row */

        /* Do the straightforward thing until (if ever) one run
         * appears to win consistently.
         */
        for (;;) {
            assert(na > 0 && nb > 1);
            k = ISLT(ssb.keys[0], ssa.keys[0]);
            if (k) {
                if (k < 0)
                    goto Fail;
                sortslice_copy_decr(&dest, &ssa);
                ++acount;
                bcount = 0;
                --na;
                if (na == 0)
                    goto Succeed;
                if (acount >= min_gallop)
                    break;
            } else {
                sortslice_copy_decr(&dest, &ssb);
                ++bcount;
                acount = 0;
                --nb;
                if (nb == 1)
                    goto CopyA;
                if (bcount >= min_gallop)
                    break;
            }
        }

        /* One run is winning so consistently that galloping may
         * be a huge win.  So try that, and continue galloping until
         * (if ever) neither run appears to be winning consistently
         * anymore.
         */
        ++min_gallop;
        do {
            assert(na > 0 && nb > 1);
            min_gallop -= min_gallop > 1;
            ms->min_gallop = min_gallop;
            k = gallop_right(ms, ssb.keys[0], basea.keys, na, na - 1);
            if (k < 0)
                goto Fail;
            k = na - k;
            acount = k;
            if (k) {
                sortslice_advance(&dest, -k);
                sortslice_advance(&ssa, -k);
                sortslice_memmove(&dest, 1, &ssa, 1, k);
                na -= k;
                if (na == 0)
                    goto Succeed;
            }
            sortslice_copy_decr(&dest, &ssb);
            --nb;
            if (nb == 1)
                goto CopyA;

            k = gallop_left(ms, ssa.keys[0], baseb.keys, nb, nb - 1);
            if (k < 0)
                goto Fail;
            k = nb - k;
            bcount = k;
            if (k) {
                sortslice_advance(&dest, -k);
                sortslice_advance(&ssb, -k);
                sortslice_memcpy(&dest, 1, &ssb, 1, k);
                nb -= k;
                if (nb == 1)
                    goto CopyA;
                /* nb==0 is impossible now if the comparison
                 * function is consistent, but we can't assume
                 * that it is.
                 */
                if (nb == 0)
                    goto Succeed;
            }
            sortslice_copy_decr(&dest, &ssa);
            --na;
            if (na == 0)
                goto Succeed;
        } while (acount >= MIN_GALLOP || bcount >= MIN_GALLOP);
        ++min_gallop;           /* penalize it for leaving galloping mode */
        ms->min_gallop = min_gallop;
    }
    Succeed:
    result = 0;
    Fail:
    if (nb)
        sortslice_memcpy(&dest, -(nb - 1), &baseb, 0, nb);
    return result;
    CopyA:
    assert(nb == 1 && na > 0);
    /* The first element of ssb belongs at the front of the merge. */
    sortslice_memmove(&dest, 1 - na, &ssa, 1 - na, na);
    sortslice_advance(&dest, -na);
    sortslice_advance(&ssa, -na);
    sortslice_copy(&dest, 0, &ssb, 0);
    return 0;
}

/* Merge the two runs at stack indices i and i+1.
 * Returns 0 on success, -1 on error.
 */
static Py_ssize_t
merge_at(MergeState *ms, Py_ssize_t i) {
    sortslice ssa, ssb;
    Py_ssize_t na, nb;
    Py_ssize_t k;

    assert(ms != NULL);
    assert(ms->n >= 2);
    assert(i >= 0);
    assert(i == ms->n - 2 || i == ms->n - 3);

    ssa = ms->pending[i].base;
    na = ms->pending[i].len;
    ssb = ms->pending[i + 1].base;
    nb = ms->pending[i + 1].len;
    assert(na > 0 && nb > 0);
    assert(ssa.keys + na == ssb.keys);

    /* Record the length of the combined runs; if i is the 3rd-last
     * run now, also slide over the last run (which isn't involved
     * in this merge).  The current run i+1 goes away in any case.
     */
    ms->pending[i].len = na + nb;
    if (i == ms->n - 3)
        ms->pending[i + 1] = ms->pending[i + 2];
    --ms->n;

    /* Where does b start in a?  Elements in a before that can be
     * ignored (already in place).
     */
    k = gallop_right(ms, *ssb.keys, ssa.keys, na, 0);
    if (k < 0)
        return -1;
    sortslice_advance(&ssa, k);
    na -= k;
    if (na == 0)
        return 0;

    /* Where does a end in b?  Elements in b after that can be
     * ignored (already in place).
     */
    nb = gallop_left(ms, ssa.keys[na - 1], ssb.keys, nb, nb - 1);
    if (nb <= 0)
        return nb;

    /* Merge what remains of the runs, using a temp array with
     * min(na, nb) elements.
     */
    if (na <= nb)
        return merge_lo(ms, ssa, na, ssb, nb);
    else
        return merge_hi(ms, ssa, na, ssb, nb);
}

/* The next run has been identified, of length n2.
 * If there's already a run on the stack, apply the "powersort" merge strategy:
 * compute the topmost run's "power" (depth in a conceptual binary merge tree)
 * and merge adjacent runs on the stack with greater power. See listsort.txt
 * for more info.
 *
 * It's the caller's responsibility to push the new run on the stack when this
 * returns.
 *
 * Returns 0 on success, -1 on error.
 */
static int
found_new_run(MergeState *ms, Py_ssize_t n2) {
    assert(ms);
    if (ms->n) {
        assert(ms->n > 0);
        struct s_slice *p = ms->pending;
        Py_ssize_t s1 = p[ms->n - 1].base.keys - ms->basekeys; /* start index */
        Py_ssize_t n1 = p[ms->n - 1].len;
        int power = powerloop(s1, n1, n2, ms->listlen);
        while (ms->n > 1 && p[ms->n - 2].power > power) {
            if (merge_at(ms, ms->n - 2) < 0)
                return -1;
        }
        assert(ms->n < 2 || p[ms->n - 2].power < power);
        p[ms->n - 1].power = power;
    }
    return 0;
}

/* Regardless of invariants, merge all runs on the stack until only one
 * remains.  This is used at the end of the mergesort.
 *
 * Returns 0 on success, -1 on error.
 */
static int
merge_force_collapse(MergeState *ms) {
    struct s_slice *p = ms->pending;

    assert(ms);
    while (ms->n > 1) {
        Py_ssize_t n = ms->n - 2;
        if (n > 0 && p[n - 1].len < p[n + 1].len)
            --n;
        if (merge_at(ms, n) < 0)
            return -1;
    }
    return 0;
}

/* Here we define custom comparison functions to optimize for the cases one commonly
 * encounters in practice: homogeneous lists, often of one of the basic types. */

/* This struct holds the comparison function and helper functions
 * selected in the pre-sort check. */

/* These are the special case compare functions.
 * ms->key_compare will always point to one of these: */

/* Heterogeneous compare: default, always safe to fall back on. */
static __forceinline int
safe_object_compare(PyObject *v, PyObject *w, MergeState *ms) {
    /* No assumptions necessary! */
    (void) ms;
    return PyObject_RichCompareBool(v, w, Py_LT);
}

/* Homogeneous compare: safe for any two comparable objects of the same type.
 * (ms->key_richcompare is set to ob_type->tp_richcompare in the
 *  pre-sort check.)
 */
static __forceinline int
unsafe_object_compare(PyObject *v, PyObject *w, MergeState *ms) {
    PyObject *res_obj;
    int res;

    /* No assumptions, because we check first: */
    if (Py_TYPE(v)->tp_richcompare != ms->key_richcompare)
        return PyObject_RichCompareBool(v, w, Py_LT);

    assert(ms->key_richcompare != NULL);
    res_obj = (*(ms->key_richcompare))(v, w, Py_LT);

    if (res_obj == Py_NotImplemented) {
        Py_DECREF(res_obj);
        return PyObject_RichCompareBool(v, w, Py_LT);
    }
    if (res_obj == NULL)
        return -1;

    if (PyBool_Check(res_obj)) {
        res = (res_obj == Py_True);
    } else {
        res = PyObject_IsTrue(res_obj);
    }
    Py_DECREF(res_obj);

    /* Note that we can't assert
     *     res == PyObject_RichCompareBool(v, w, Py_LT);
     * because of evil compare functions like this:
     *     lambda a, b:  int(random.random() * 3) - 1)
     * (which is actually in test_sort.py) */
    return res;
}

/* Latin string compare: safe for any two latin (one byte per char) strings. */
static __forceinline int
unsafe_latin_compare(PyObject *v, PyObject *w, MergeState *ms) {
    (void) ms;
    Py_ssize_t len;
    int res;

    /* Modified from Objects/unicodeobject.c:unicode_compare, assuming: */
    assert(Py_IS_TYPE(v, &PyUnicode_Type));
    assert(Py_IS_TYPE(w, &PyUnicode_Type));
    assert(PyUnicode_KIND(v) == PyUnicode_KIND(w));
    assert(PyUnicode_KIND(v) == PyUnicode_1BYTE_KIND);

    len = Py_MIN(PyUnicode_GET_LENGTH(v), PyUnicode_GET_LENGTH(w));
    res = memcmp(PyUnicode_DATA(v), PyUnicode_DATA(w), len);

    res = (res != 0 ?
           res < 0 :
           PyUnicode_GET_LENGTH(v) < PyUnicode_GET_LENGTH(w));

    assert(res == PyObject_RichCompareBool(v, w, Py_LT));
    return res;
}

/* Bounded int compare: compare any two longs that fit in a single machine word. */
static __forceinline int
unsafe_long_compare(PyObject *v, PyObject *w, MergeState *ms) {
    (void) ms;
    intptr_t v0, w0;
    int res;

    /* Modified from Objects/longobject.c:long_compare, assuming: */
    assert(Py_IS_TYPE(v, &PyLong_Type));
    assert(Py_IS_TYPE(w, &PyLong_Type));
#ifdef IS_PYTHON_312_OR_LATER
    assert(_PyLong_IsCompact((PyLongObject *) v));
    assert(_PyLong_IsCompact((PyLongObject *) w));
#endif

#ifdef IS_PYTHON_312_OR_LATER
    v0 = _PyLong_CompactValue((PyLongObject *) v);
    w0 = _PyLong_CompactValue((PyLongObject *) w);
#else
    v0 = PyLong_AsSsize_t(v);
    w0 = PyLong_AsSsize_t(w);
#endif

    res = v0 < w0;
    assert(res == PyObject_RichCompareBool(v, w, Py_LT));
    return res;
}

/* Float compare: compare any two floats. */
static __forceinline int
unsafe_float_compare(PyObject *v, PyObject *w, MergeState *ms) {
    (void) ms;
    int res;

    /* Modified from Objects/floatobject.c:float_richcompare, assuming: */
    assert(Py_IS_TYPE(v, &PyFloat_Type));
    assert(Py_IS_TYPE(w, &PyFloat_Type));

    res = PyFloat_AS_DOUBLE(v) < PyFloat_AS_DOUBLE(w);
    assert(res == PyObject_RichCompareBool(v, w, Py_LT));
    return res;
}

/* Tuple compare: compare *any* two tuples, using
 * ms->tuple_elem_compare to compare the first elements, which is set
 * using the same pre-sort check as we use for ms->key_compare,
 * but run on the list [x[0] for x in L]. This allows us to optimize compares
 * on two levels (as long as [x[0] for x in L] is type-homogeneous.) The idea is
 * that most tuple compares don't involve x[1:]. */
static __forceinline int
unsafe_tuple_compare(PyObject *v, PyObject *w, MergeState *ms) {
    PyTupleObject *vt, *wt;
    Py_ssize_t i, vlen, wlen;
    int k;

    /* Modified from Objects/tupleobject.c:tuplerichcompare, assuming: */
    assert(Py_IS_TYPE(v, &PyTuple_Type));
    assert(Py_IS_TYPE(w, &PyTuple_Type));
    assert(Py_SIZE(v) > 0);
    assert(Py_SIZE(w) > 0);

    vt = (PyTupleObject *) v;
    wt = (PyTupleObject *) w;

    vlen = Py_SIZE(vt);
    wlen = Py_SIZE(wt);

    for (i = 0; i < vlen && i < wlen; i++) {
        k = PyObject_RichCompareBool(vt->ob_item[i], wt->ob_item[i], Py_EQ);
        if (k < 0)
            return -1;
        if (!k)
            break;
    }

    if (i >= vlen || i >= wlen)
        return vlen < wlen;

    if (i == 0)
        return ms->tuple_elem_compare(vt->ob_item[i], wt->ob_item[i], ms);
    else
        return PyObject_RichCompareBool(vt->ob_item[i], wt->ob_item[i], Py_LT);
}

/* An adaptive, stable, natural mergesort.  See listsort.txt.
 * Returns Py_None on success, NULL on error.  Even in case of error, the
 * list will be some permutation of its input state (nothing is lost or
 * duplicated).
 */
/*[clinic input]
@critical_section
list.sort

    *
    key as keyfunc: object = None
    reverse: bool = False

Sort the list in ascending order and return None.

The sort is in-place (i.e. the list itself is modified) and stable (i.e. the
order of two equal elements is maintained).

If a key function is given, apply it once to each list item and sort them,
ascending or descending, according to their function values.

The reverse flag can be set to sort in descending order.
[clinic start generated code]*/
PyObject *CPython_sort(PyObject **items, Py_ssize_t size, PyObject *keyfunc, int reverse)
/*[clinic end generated code: output=57b9f9c5e23fbe42 input=667bf25d0e3a3676]*/
{
    MergeState ms;
    Py_ssize_t nremaining;
    Py_ssize_t minrun;
    sortslice lo;
    Py_ssize_t saved_ob_size;
    PyObject **saved_ob_item;
    PyObject **final_ob_item;
    PyObject *result = NULL;            /* guilty until proved innocent */
    Py_ssize_t i;
    PyObject **keys;

    assert(items != NULL);
    if (keyfunc == Py_None)
        keyfunc = NULL;

    /* The list is temporarily made empty, so that mutations performed
     * by comparison functions can't affect the slice of memory we're
     * sorting (allowing mutations during sorting is a core-dump
     * factory, since ob_item may change).
     */
    saved_ob_size = size;
    saved_ob_item = items;
    size = 0;
    items = NULL;

    if (keyfunc == NULL) {
        keys = NULL;
        lo.keys = saved_ob_item;
        lo.values = NULL;
    } else {
        if (saved_ob_size < MERGESTATE_TEMP_SIZE / 2)
            /* Leverage stack space we allocated but won't otherwise use */
            keys = &ms.temparray[saved_ob_size + 1];
        else {
            keys = PyMem_Malloc(sizeof(PyObject *) * saved_ob_size);
            if (keys == NULL) {
                PyErr_NoMemory();
                goto keyfunc_fail;
            }
        }

        for (i = 0; i < saved_ob_size; i++) {
            keys[i] = PyObject_CallOneArg(keyfunc, saved_ob_item[i]);
            if (keys[i] == NULL) {
                for (i = i - 1; i >= 0; i--)
                    Py_DECREF(keys[i]);
                if (saved_ob_size >= MERGESTATE_TEMP_SIZE / 2)
                    PyMem_Free(keys);
                goto keyfunc_fail;
            }
        }

        lo.keys = keys;
        lo.values = saved_ob_item;
    }


    /* The pre-sort check: here's where we decide which compare function to use.
     * How much optimization is safe? We test for homogeneity with respect to
     * several properties that are expensive to check at compare-time, and
     * set ms appropriately. */
    if (saved_ob_size > 1) {
        /* Assume the first element is representative of the whole list. */
        int keys_are_in_tuples = (Py_IS_TYPE(lo.keys[0], &PyTuple_Type) &&
                                  Py_SIZE(lo.keys[0]) > 0);

        PyTypeObject *key_type = (keys_are_in_tuples ?
                                  Py_TYPE(PyTuple_GET_ITEM(lo.keys[0], 0)) :
                                  Py_TYPE(lo.keys[0]));

        int keys_are_all_same_type = 1;
        int strings_are_latin = 1;
        int ints_are_bounded = 1;

        /* Prove that assumption by checking every key. */
        for (i = 0; i < saved_ob_size; i++) {

            if (keys_are_in_tuples &&
                !(Py_IS_TYPE(lo.keys[i], &PyTuple_Type) && Py_SIZE(lo.keys[i]) != 0)) {
                keys_are_in_tuples = 0;
                keys_are_all_same_type = 0;
                break;
            }

            /* Note: for lists of tuples, key is the first element of the tuple
             * lo.keys[i], not lo.keys[i] itself! We verify type-homogeneity
             * for lists of tuples in the if-statement directly above. */
            PyObject *key = (keys_are_in_tuples ?
                             PyTuple_GET_ITEM(lo.keys[i], 0) :
                             lo.keys[i]);

            if (!Py_IS_TYPE(key, key_type)) {
                keys_are_all_same_type = 0;
                /* If keys are in tuple we must loop over the whole list to make
                   sure all items are tuples */
                if (!keys_are_in_tuples) {
                    break;
                }
            }

            if (keys_are_all_same_type) {
                if (key_type == &PyLong_Type &&
                    ints_are_bounded
#ifdef IS_PYTHON_312_OR_LATER
                    && !_PyLong_IsCompact((PyLongObject *) key)
#endif
                    ) {

                    ints_are_bounded = 0;
                } else if (key_type == &PyUnicode_Type &&
                           strings_are_latin &&
                           PyUnicode_KIND(key) != PyUnicode_1BYTE_KIND) {

                    strings_are_latin = 0;
                }
            }
        }

        /* Choose the best compare, given what we now know about the keys. */
        if (keys_are_all_same_type) {

            if (key_type == &PyUnicode_Type && strings_are_latin) {
                ms.key_compare = unsafe_latin_compare;
            } else if (key_type == &PyLong_Type && ints_are_bounded) {
                ms.key_compare = unsafe_long_compare;
            } else if (key_type == &PyFloat_Type) {
                ms.key_compare = unsafe_float_compare;
            } else if ((ms.key_richcompare = key_type->tp_richcompare) != NULL) {
                ms.key_compare = unsafe_object_compare;
            } else {
                ms.key_compare = safe_object_compare;
            }
        } else {
            ms.key_compare = safe_object_compare;
        }

        if (keys_are_in_tuples) {
            /* Make sure we're not dealing with tuples of tuples
             * (remember: here, key_type refers list [key[0] for key in keys]) */
            if (key_type == &PyTuple_Type) {
                ms.tuple_elem_compare = safe_object_compare;
            } else {
                ms.tuple_elem_compare = ms.key_compare;
            }

            ms.key_compare = unsafe_tuple_compare;
        }
    }
    /* End of pre-sort check: ms is now set properly! */

    merge_init(&ms, saved_ob_size, keys != NULL, &lo);

    nremaining = saved_ob_size;
    if (nremaining < 2)
        goto succeed;

    /* Reverse sort stability achieved by initially reversing the list,
    applying a stable forward sort, then reversing the final result. */
    if (reverse) {
        if (keys != NULL)
            reverse_slice(&keys[0], &keys[saved_ob_size]);
        reverse_slice(&saved_ob_item[0], &saved_ob_item[saved_ob_size]);
    }

    /* March over the array once, left to right, finding natural runs,
     * and extending short natural runs to minrun elements.
     */
    minrun = merge_compute_minrun(nremaining);
    do {
        Py_ssize_t n;

        /* Identify next run. */
        n = count_run(&ms, &lo, nremaining);
        if (n < 0)
            goto fail;
        /* If short, extend to min(minrun, nremaining). */
        if (n < minrun) {
            const Py_ssize_t force = nremaining <= minrun ?
                                     nremaining : minrun;
            if (binarysort(&ms, &lo, force, n) < 0)
                goto fail;
            n = force;
        }
        /* Maybe merge pending runs. */
        assert(ms.n == 0 || ms.pending[ms.n - 1].base.keys +
                            ms.pending[ms.n - 1].len == lo.keys);
        if (found_new_run(&ms, n) < 0)
            goto fail;
        /* Push new run on stack. */
        assert(ms.n < MAX_MERGE_PENDING);
        ms.pending[ms.n].base = lo;
        ms.pending[ms.n].len = n;
        ++ms.n;
        /* Advance to find next run. */
        sortslice_advance(&lo, n);
        nremaining -= n;
    } while (nremaining);

    if (merge_force_collapse(&ms) < 0)
        goto fail;
    assert(ms.n == 1);
    assert(keys == NULL
           ? ms.pending[0].base.keys == saved_ob_item
           : ms.pending[0].base.keys == &keys[0]);
    assert(ms.pending[0].len == saved_ob_size);
    lo = ms.pending[0].base;

    succeed:
    result = Py_None;
    fail:
    if (keys != NULL) {
        for (i = 0; i < saved_ob_size; i++)
            Py_DECREF(keys[i]);
        if (saved_ob_size >= MERGESTATE_TEMP_SIZE / 2)
            PyMem_Free(keys);
    }

    if (reverse && saved_ob_size > 1)
        reverse_slice(saved_ob_item, saved_ob_item + saved_ob_size);

    merge_freemem(&ms);

    keyfunc_fail:
    final_ob_item = items;
    i = size;
    size = saved_ob_size;
    items = saved_ob_item;
    if (final_ob_item != NULL) {
        /* we cannot use list_clear() for this because it does not
           guarantee that the list is really empty when it returns */
        while (--i >= 0) {
            Py_XDECREF(final_ob_item[i]);
        }
#ifdef Py_GIL_DISABLED
        bool use_qsbr = _PyObject_GC_IS_SHARED(self);
#else
        bool use_qsbr = false;
#endif
        free_list_items(final_ob_item, use_qsbr);
    }
    return Py_XNewRef(result);
}

#undef IFLT
#undef ISLT
