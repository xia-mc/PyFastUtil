//
// Created by xia__mc on 2024/11/11.
//

#ifndef PYFASTUTIL_ALIGNEDALLOCATOR_H
#define PYFASTUTIL_ALIGNEDALLOCATOR_H

#include <cstdlib>

template <typename T, std::size_t Alignment>
class AlignedAllocator {
public:
    using value_type [[maybe_unused]] = T;
    using pointer [[maybe_unused]] = T*;
    using const_pointer [[maybe_unused]] = const T*;
    using reference [[maybe_unused]] = T&;
    using const_reference [[maybe_unused]] = const T&;
    using size_type [[maybe_unused]] = std::size_t;
    using difference_type [[maybe_unused]] = std::ptrdiff_t;

    AlignedAllocator() = default;

    template <class U>
    explicit AlignedAllocator(const AlignedAllocator<U, Alignment>&) {}

    [[maybe_unused]] __forceinline T* allocate(std::size_t n) {
        void* ptr;

#ifdef _WIN32
        ptr = _aligned_malloc(n * sizeof(T), Alignment);
#else
        ptr = std::aligned_alloc(Alignment, n * sizeof(T));
#endif

        if (ptr == nullptr)
            throw std::bad_alloc();
        return static_cast<T*>(ptr);
    }

    [[maybe_unused]] __forceinline void deallocate(T* ptr, std::size_t) noexcept {
#ifdef _WIN32
        _aligned_free(ptr);
#else
        std::free(ptr);
#endif
    }

    // Rebind allocator to another type
    template <typename U>
    struct [[maybe_unused]] rebind {
        using other [[maybe_unused]] = AlignedAllocator<U, Alignment>;
    };
};

template <typename T, std::size_t Alignment, typename U>
__forceinline bool operator==(const AlignedAllocator<T, Alignment>&, const AlignedAllocator<U, Alignment>&) {
    return true;
}

template <typename T, std::size_t Alignment, typename U>
__forceinline bool operator!=(const AlignedAllocator<T, Alignment>& lhs, const AlignedAllocator<U, Alignment>& rhs) {
    return !(lhs == rhs);
}

#endif //PYFASTUTIL_ALIGNEDALLOCATOR_H