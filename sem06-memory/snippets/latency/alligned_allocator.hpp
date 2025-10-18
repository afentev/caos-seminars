#include <cstdlib>

template <typename T, size_t Alignment>
class aligned_allocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    template <typename U>
    struct rebind {
        using other = aligned_allocator<U, Alignment>;
    };

    aligned_allocator() = default;
    template <typename U>
    aligned_allocator(const aligned_allocator<U, Alignment>&) {}

    pointer allocate(size_type n) {
        if (n > std::numeric_limits<size_type>::max() / sizeof(T)) {
            throw std::bad_alloc();
        }
        size_type bytes = n * sizeof(T);
        void* ptr = nullptr;
        if (posix_memalign(&ptr, Alignment, bytes) != 0) {
            throw std::bad_alloc();
        }
        return static_cast<pointer>(ptr);
    }

    void deallocate(pointer p, size_type) {
        std::free(p);
    }
};

template <typename T, size_t Alignment>
bool operator==(const aligned_allocator<T, Alignment>&, const aligned_allocator<T, Alignment>&) {
    return true;
}

template <typename T, size_t Alignment>
bool operator!=(const aligned_allocator<T, Alignment>&, const aligned_allocator<T, Alignment>&) {
    return false;
}