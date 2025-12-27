#pragma once

#define MEMORY_IMPLEMENTATION

// #include <new>

#include "Json/object.h"

namespace qlib {

namespace memory {

template <class T1, class T2>
NODISCARD FORCE_INLINE static constexpr auto align_up(T1 size, T2 alignment) noexcept {
    return (size + (alignment - 1)) & ~(alignment - 1);
}

class bad_alloc final : public exception {
public:
    char const* what() const noexcept override { return "bad alloc"; }
};

template <class T>
class reference : public object {
public:
    using base = object;
    using self = reference;
    using value_type = T;

protected:
    value_type* _impl;

public:
    INLINE constexpr reference() noexcept = delete;
    INLINE constexpr reference(reference const&) noexcept = default;
    INLINE constexpr reference(reference&&) noexcept = default;
    INLINE constexpr reference& operator=(reference const&) noexcept = default;
    INLINE constexpr reference& operator=(reference&&) noexcept = default;
    INLINE ~reference() noexcept = default;

    INLINE constexpr reference(value_type& allocator) noexcept : _impl(&allocator) {}
    INLINE constexpr reference(value_type* allocator) noexcept : _impl(allocator) {}
    NODISCARD INLINE constexpr operator value_type&() noexcept { return *_impl; }
};

class new_allocator : public object {
public:
    using reference = new_allocator;

    template <class T>
    NODISCARD INLINE static constexpr T* allocate(uint64_t n) noexcept {
        static_assert(sizeof(T) > 0, "cannot allocate zero-sized object");
        return (T*)(::operator new(n * sizeof(T)));
    }

    template <class T>
    INLINE static constexpr void deallocate(T* p, uint64_t n __attribute__((__unused__))) noexcept {
        ::operator delete(p
#if __cpp_sized_deallocation
                          ,
                          n * sizeof(T)
#endif
        );
    }

    template <class T, class... Args>
    INLINE static constexpr void construct(T* p, Args&&... args) noexcept(
        is_nothrow_constructible_v<T, Args...>) {
        new (p) T(forward<Args>(args)...);
    }

    template <class T>
    INLINE static constexpr void destroy(T* p) noexcept(is_nothrow_destructible_v<T>) {
        p->~T();
    }

    NODISCARD INLINE static constexpr uint64_t max_size() noexcept { return (uint64_t)(-1); }
};

class pool_allocator final : public new_allocator {
public:
    using base = new_allocator;
    using self = pool_allocator;
    using size_type = uint64_t;

protected:
    struct alignas(8) node {
        node* next{nullptr};
        uint8_t* data{nullptr};
        size_type used{0u};
        size_type capacity{0u};
    };

    node* _list{nullptr};

public:
    using reference = memory::reference<self>;

    INLINE pool_allocator(size_type capacity = 64 * 1024) noexcept {
        _list = (node*)::operator new(sizeof(node) + capacity);
        _list->next = nullptr;
        _list->data = (uint8_t*)_list + sizeof(node);
        _list->used = 0u;
        _list->capacity = capacity;
    }

    INLINE ~pool_allocator() noexcept {
        node* cur = _list;
        while (cur != nullptr) {
            node* next = cur->next;
            ::operator delete(cur
#if __cpp_sized_deallocation
                              ,
                              sizeof(node) + cur->capacity
#endif
            );
            cur = next;
        }
    }

    template <class T>
    NODISCARD INLINE constexpr T* allocate(size_type n) noexcept {
        static_assert(sizeof(T) > 0, "cannot allocate zero-sized object");

        size_type size = align_up(sizeof(T) * n, sizeof(void*));

        if (unlikely(_list->used + size > _list->capacity)) {
            // ++_pool_count;
            size_type new_capacity = _list->capacity * 2;
            while (new_capacity < size) {
                new_capacity *= 2;
            }
            auto new_node = (node*)(::operator new(sizeof(node) + new_capacity));
            new_node->next = _list;
            new_node->data = (uint8_t*)new_node + sizeof(node);
            new_node->used = 0u;
            new_node->capacity = new_capacity;
            _list = new_node;
        }

        auto ptr = (T*)(_list->data + _list->used);
        _list->used += size;
        return ptr;
    }

    template <class T>
    INLINE constexpr void deallocate(T* p __attribute__((__unused__)),
                                     uint64_t n __attribute__((__unused__))) noexcept {}
};

template <uint64_t Capacity>
class stack_allocator final : public new_allocator {
public:
    using base = new_allocator;
    using self = stack_allocator;
    using size_type = uint64_t;

protected:
    size_type _used{0u};
    alignas(8) uint8_t _impl[Capacity]{0u};

public:
    using reference = memory::reference<self>;

    template <class T>
    NODISCARD INLINE constexpr T* allocate(size_type n) noexcept {
        static_assert(sizeof(T) > 0, "cannot allocate zero-sized object");
        size_type size = align_up(sizeof(T) * n, sizeof(void*));
        throw_if(_used + size > Capacity, bad_alloc());
        auto ptr = static_cast<T*>(static_cast<void*>(_impl + _used));
        _used += size;
        return ptr;
    }

    template <class T>
    INLINE constexpr void deallocate(T* p __attribute__((__unused__)),
                                     uint64_t n __attribute__((__unused__))) noexcept {}
};

};  // namespace memory

using new_allocator_t = memory::new_allocator;
using pool_allocator_t = memory::pool_allocator;
template <uint64_t Capacity = 64 * 1024>
using stack_allocator_t = memory::stack_allocator<Capacity>;

};  // namespace qlib
