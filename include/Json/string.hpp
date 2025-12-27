#pragma once

#define STRING_IMPLEMENTATION

#include "Json/memory.h"
#include "Json/object.h"

namespace qlib {

namespace string {

template <class Char>
NODISCARD INLINE constexpr uint64_t strlen(Char const* str) noexcept {
    uint64_t size{0u};
    while (str[size] != '\0') {
        ++size;
    }
    return size;
}

template <class Char>
NODISCARD INLINE constexpr bool_t in(Char c, Char const* str) noexcept {
    bool_t ok{False};
    while (likely(*str != '\0')) {
        if (unlikely(c == *str)) {
            ok = True;
            break;
        }
        ++str;
    }
    return ok;
}

template <class T, class Iter>
INLINE CONSTEXPR enable_if_t<is_unsigned_v<T>, void> __to_chars(Iter __last, T __value) noexcept {
    constexpr char __digits[201] = "0001020304050607080910111213141516171819"
                                   "2021222324252627282930313233343536373839"
                                   "4041424344454647484950515253545556575859"
                                   "6061626364656667686970717273747576777879"
                                   "8081828384858687888990919293949596979899";
    while (__value >= 100) {
        auto const __num = (__value % 100) * 2;
        __value /= 100;
        *--__last = __digits[__num + 1];
        *--__last = __digits[__num];
    }
    if (__value >= 10) {
        auto const __num = __value * 2;
        *--__last = __digits[__num + 1];
        *--__last = __digits[__num];
    } else {
        *--__last = '0' + __value;
    }
}

template <class T, class Iter1, class Iter2>
INLINE CONSTEXPR enable_if_t<is_unsigned_v<T>, Iter1> to_chars(Iter1 __first,
                                                               Iter2 __last,
                                                               T __value) noexcept {
    auto __len = len(__value);
    throw_if(__len > decltype(__len)(distance(__first, __last)));

    __last = __first + __len;
    __to_chars<T>(__last, __value);

    return __last;
}

template <class T, class Iter1, class Iter2>
INLINE CONSTEXPR enable_if_t<is_signed_v<T>, Iter1> to_chars(Iter1 __first,
                                                             Iter2 __last,
                                                             T __value) noexcept {
    using U = typename unsigned_traits<T>::type;
    U __val = __value;
    size_t __len{0u};
    if (__value < 0) {
        __val = U(~__value) + U(1);
        __len = 1u;
    }
    __len += len(__val);
    throw_if(__len > decltype(__len)(distance(__first, __last)));

    *__first = '-';
    __last = __first + __len;
    __to_chars<U>(__last, __val);

    return __last;
}

template <class T, class Iter1, class Iter2, size_t Precision = 6>
INLINE CONSTEXPR enable_if_t<is_floating_point_v<T>, Iter1> to_chars(Iter1 __first,
                                                                     Iter2 __last,
                                                                     T __value) noexcept {
    throw_if(__builtin_isnan(__value) || __builtin_isinf(__value));

    size_t __len{0u};
    T __val = __value;
    if (__value < 0) {
        __val = -__value;
        __len = 1u;
    }

    auto value_int = static_cast<uint64_t>(__val);
    __len += len(value_int);
    throw_if(__len + 1u + Precision > decltype(__len)(distance(__first, __last)));

    *__first = '-';
    __first = __first + __len;
    __to_chars<uint64_t>(__first, value_int);

    auto value_frac = __val - value_int;
    *__first++ = '.';
    for (auto i = 0u; i < Precision - 1; ++i) {
        value_frac *= 10u;
        auto digit = static_cast<uint8_t>(value_frac);
        *__first++ = static_cast<char>(digit + '0');
        value_frac -= digit;
    }
    value_frac *= 10u;
    auto digit = static_cast<uint8_t>(value_frac);
    value_frac -= digit;
    if (value_frac > 0.5) {
        digit += 1;
    }
    *__first++ = static_cast<char>(digit + '0');
    return __first;
}

class bad_to final : public exception {
public:
    char const* what() const noexcept override { return "bad to"; }
};

class bad_from final : public exception {
public:
    char const* what() const noexcept override { return "bad from"; }
};

class out_of_range final : public exception {
public:
    char const* what() const noexcept override { return "out of range"; }
};

template <class Char>
struct char_traits final : public object {
    using value_type = Char;
    using pointer = value_type*;
    using const_pointer = value_type const*;
    using reference = value_type&;
    using const_reference = value_type const&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using size_type = uint32_t;
    static constexpr size_type npos = size_type(-1);
};

template <class Char>
class view final : public object {
public:
    using self = view;
    using char_type = Char;
    using traits_type = char_traits<Char>;
    using value_type = typename traits_type::value_type;
    using const_pointer = typename traits_type::const_pointer;
    using const_reference = typename traits_type::const_reference;
    using const_iterator = typename traits_type::const_iterator;
    using size_type = typename traits_type::size_type;
    static constexpr size_type npos = traits_type::npos;

protected:
    const_pointer _impl{nullptr};
    size_type _size{0u};

public:
    INLINE constexpr view() noexcept = default;

    template <class Iter1, class Iter2>
    INLINE explicit constexpr view(Iter1 begin, Iter2 end) noexcept
            : _impl(begin), _size(distance(begin, end)) {}

    INLINE constexpr view(const_pointer o) noexcept : _impl(o), _size(strlen(o)) {}

    INLINE constexpr view(self const& o) noexcept : _impl(o._impl), _size(o._size) {}

    INLINE constexpr self& operator=(const_pointer o) noexcept {
        _impl = o;
        _size = strlen(o);
        return *this;
    }

    INLINE constexpr self& operator=(self const& o) noexcept {
        _impl = o._impl;
        _size = o._size;
        return *this;
    }

    NODISCARD INLINE constexpr const_pointer data() const noexcept { return _impl; }

    NODISCARD INLINE constexpr size_type size() const noexcept { return _size; }

    NODISCARD INLINE constexpr self substr(size_type pos, size_type n = npos) const noexcept {
        // if (unlikely(pos > size())) {
        //     return self();
        // }
        size_type len = (n == npos || pos + n > size()) ? size() - pos : n;
        return self(data() + pos, data() + pos + len);
    }

    NODISCARD INLINE constexpr bool_t starts_with(const_pointer o) const noexcept {
        return starts_with(self(o));
    }

    NODISCARD INLINE constexpr bool_t starts_with(self const& o) const noexcept {
        return substr(0u, o.size()) == o;
    }

    NODISCARD INLINE constexpr bool_t ends_with(const_pointer o) const noexcept {
        return ends_with(self(o));
    }

    NODISCARD INLINE constexpr bool_t ends_with(self const& o) const noexcept {
        return size() >= o.size() && substr(size() - o.size(), o.size()) == o;
    }

    NODISCARD INLINE constexpr bool_t operator==(const_pointer o) const noexcept {
        return *this == self(o);
    }

    NODISCARD INLINE constexpr bool_t operator==(self const& o) const noexcept {
        return size() == o.size() && equal(begin(), end(), o.begin());
    }

    template <class T>
    NODISCARD INLINE constexpr bool_t operator!=(T o) const noexcept {
        return !(*this == o);
    }

    NODISCARD INLINE constexpr auto operator[](size_type pos) const noexcept {
        return *(data() + pos);
    }

    NODISCARD INLINE constexpr auto empty() const noexcept { return size() == 0u; }

    NODISCARD INLINE constexpr auto begin() const noexcept { return data(); }

    NODISCARD INLINE constexpr auto end() const noexcept { return data() + size(); }

    NODISCARD INLINE constexpr auto front() const noexcept { return *(data()); }

    NODISCARD INLINE constexpr auto back() const noexcept { return *(data() + size() - 1); }

#ifdef _GLIBCXX_STRING_VIEW
    NODISCARD INLINE operator std::basic_string_view<value_type>() const noexcept {
        return std::basic_string_view<value_type>(data(), size());
    }
#endif

    NODISCARD INLINE explicit operator bool_t() const noexcept { return !empty(); }
};

template <class Char, class Allocator = new_allocator_t>
class value final : public Allocator::reference {
public:
    using base = typename Allocator::reference;
    using self = value;
    using char_type = Char;
    using allocator_type = Allocator;
    using view_type = view<Char>;
    using traits_type = char_traits<Char>;
    using value_type = typename traits_type::value_type;
    using pointer = typename traits_type::pointer;
    using const_pointer = typename traits_type::const_pointer;
    using reference = typename traits_type::reference;
    using const_reference = typename traits_type::const_reference;
    using iterator = typename traits_type::iterator;
    using const_iterator = typename traits_type::const_iterator;
    using size_type = typename traits_type::size_type;
    static constexpr size_type npos = traits_type::npos;

protected:
    Char* _impl{nullptr};
    size_type _size{0u};
    size_type _capacity{0u};

    constexpr allocator_type& _allocator() noexcept {
        using reference = typename allocator_type::reference;
        return static_cast<reference&>(*this);
    }
    constexpr allocator_type const& _allocator() const noexcept {
        using reference = typename allocator_type::reference;
        return static_cast<reference&>(*this);
    }

    template <class Iter1, class Iter2>
    INLINE constexpr void _init(Iter1 begin, Iter2 end) {
        const size_type size = distance(begin, end);
        _impl = _allocator().template allocate<Char>(size + 1u);
        _capacity = size;
        copy(begin, end, _impl);
        _impl[size] = '\0';
        _size = size;
        // }
    }

    template <class Iter1, class Iter2>
    INLINE constexpr void _assign(Iter1 begin, Iter2 end) {
        const size_type size = distance(begin, end);
        // if (likely(size > 0)) {
        if (likely(size > capacity())) {
            _allocator().template deallocate<Char>(_impl, capacity());
            _impl = _allocator().template allocate<Char>(size + 1u);
            _capacity = size;
        }
        copy(begin, end, _impl);
        _impl[size] = '\0';
        _size = size;
        // }
    }

public:
    INLINE constexpr value() noexcept(is_nothrow_constructible_v<base>) {}

    INLINE constexpr explicit value(allocator_type& allocator) noexcept(
        is_nothrow_constructible_v<base>)
            : base(allocator) {}

    INLINE constexpr explicit value(size_type capacity)
            : _impl{_allocator().template allocate<Char>(capacity + 1u)}, _capacity(capacity) {}

    INLINE constexpr explicit value(size_type capacity, allocator_type& allocator)
            : base(allocator), _impl{_allocator().template allocate<Char>(capacity + 1u)},
              _capacity(capacity) {}

    template <
        class Iter1,
        class Iter2,
        class Enable = enable_if_t<is_same_v<typename iterator_traits<Iter1>::value_type, Char> &&
                                   is_same_v<typename iterator_traits<Iter2>::value_type, Char>>>
    INLINE constexpr explicit value(Iter1 begin, Iter2 end) {
        _init(begin, end);
    }

    template <
        class Iter1,
        class Iter2,
        class Enable = enable_if_t<is_same_v<typename iterator_traits<Iter1>::value_type, Char> &&
                                   is_same_v<typename iterator_traits<Iter2>::value_type, Char>>>
    INLINE constexpr explicit value(Iter1 begin, Iter2 end, allocator_type& allocator)
            : base(allocator) {
        _init(begin, end);
    }

    INLINE constexpr value(const_pointer str) : value(str, str + strlen(str)) {}

    INLINE constexpr value(const_pointer str, allocator_type& allocator)
            : value(str, str + strlen(str), allocator) {}

    INLINE constexpr value(view_type o) : value(o.begin(), o.end()) {}

    INLINE constexpr value(view_type o, allocator_type& allocator)
            : value(o.begin(), o.end(), allocator) {}

    INLINE constexpr value(self const& o) : base(o) { _init(o.begin(), o.end()); }

    INLINE constexpr value(self&& o) noexcept
            : base(move(o)), _impl(o._impl), _size(o._size), _capacity(o._capacity) {
        o._impl = nullptr;
    }

#ifdef _GLIBCXX_STRING_VIEW
    INLINE constexpr explicit value(std::basic_string_view<Char> str)
            : value(str.data(), str.data() + str.size()) {}
#endif

    INLINE ~value() noexcept(is_nothrow_destructible_v<base>) {
        if (_impl != nullptr) {
            _allocator().template deallocate<Char>(_impl, _capacity);
            _impl = nullptr;
            _size = 0u;
            _capacity = 0u;
        }
    }

    INLINE constexpr self& operator=(const_pointer o) {
        _assign(o, o + strlen(o));
        return *this;
    }

    INLINE constexpr self& operator=(view_type o) {
        _assign(o.begin(), o.end());
        return *this;
    }

    INLINE constexpr self& operator=(self const& o) {
        if (unlikely(this != &o)) {
            _assign(o.begin(), o.end());
        }
        return *this;
    }

    INLINE constexpr self& operator=(self&& o) noexcept {
        _allocator().template deallocate<Char>(_impl, _capacity);
        _impl = o._impl;
        _size = o._size;
        _capacity = o._capacity;
        o._impl = nullptr;
        o._size = 0u;
        o._capacity = 0u;
        return *this;
    }

    INLINE void reserve(size_type capacity) {
        if (capacity > _capacity) {
            auto impl = _allocator().template allocate<Char>(capacity + 1u);
            copy(begin(), end(), impl);
            _allocator().template deallocate<Char>(_impl, _capacity);
            _impl = impl;
            _impl[_size] = '\0';
            _capacity = capacity;
        }
    }

    NODISCARD INLINE constexpr pointer data() noexcept { return _impl; }

    NODISCARD INLINE constexpr const_pointer data() const noexcept { return _impl; }

    NODISCARD INLINE constexpr size_type size() const noexcept { return _size; }

    NODISCARD INLINE constexpr size_type capacity() const noexcept { return _capacity; }

    NODISCARD INLINE constexpr bool_t empty() const noexcept { return size() == 0u; }

    NODISCARD INLINE constexpr reference operator[](size_type pos) { return data()[pos]; }

    NODISCARD INLINE constexpr const_reference operator[](size_type pos) const {
        return const_cast<self&>(*this)[pos];
    }

    NODISCARD INLINE constexpr iterator begin() noexcept { return data(); }

    NODISCARD INLINE constexpr const_iterator begin() const noexcept { return data(); }

    NODISCARD INLINE constexpr iterator end() noexcept { return data() + size(); }

    NODISCARD INLINE constexpr const_iterator end() const noexcept { return data() + size(); }

    NODISCARD INLINE constexpr reference front() noexcept { return *data(); }
    NODISCARD INLINE constexpr const_reference front() const noexcept {
        return const_cast<self&>(*this).front();
    }
    NODISCARD INLINE constexpr reference back() noexcept { return *(data() + size() - 1); }
    NODISCARD INLINE constexpr const_reference back() const noexcept {
        return const_cast<self&>(*this).back();
    }

    NODISCARD INLINE constexpr const_pointer c_str() const noexcept { return data(); }

    NODISCARD INLINE constexpr bool_t starts_with(self const& o) const noexcept {
        return static_cast<view_type>(*this).starts_with(o);
    }

    NODISCARD INLINE constexpr bool_t ends_with(self const& o) const noexcept {
        return static_cast<view_type>(*this).ends_with(o);
    }

    NODISCARD INLINE constexpr bool_t operator==(view_type o) const noexcept {
        return size() == o.size() && equal(begin(), end(), o.begin());
    }

    template <class T>
    NODISCARD INLINE constexpr bool_t operator!=(T o) const noexcept {
        return !(*this == o);
    }

    INLINE CONSTEXPR void clear() noexcept { _size = 0u; }

    template <class Iter1, class Iter2>
    INLINE void emplace(Iter1 __begin, Iter2 __end) noexcept {
        const size_type __size = distance(__begin, __end);
        // if (likely(__size > 0u)) {
        const size_type __new_size = size() + __size;
        if (unlikely(__new_size > capacity())) {
            _allocator().template deallocate<Char>(_impl, _capacity);
            size_type const __new_capacity = __new_size * 2;
            auto __impl = _allocator().template allocate<Char>(__new_capacity + 1u);
            copy(begin(), end(), __impl);
            _impl = __impl;
            _capacity = __new_capacity;
        }
        copy(__begin, __end, end());
        _impl[__new_size] = '\0';
        _size = __new_size;
        // }
    }

    INLINE self& operator<<(Char ch) {
        emplace(&ch, &ch + 1);
        return *this;
    }

    INLINE self& operator<<(const_pointer o) {
        emplace(o, o + strlen(o));
        return *this;
    }

    INLINE self& operator<<(view_type o) {
        emplace(o.begin(), o.end());
        return *this;
    }

    INLINE self& operator<<(self const& o) {
        emplace(o.begin(), o.end());
        return *this;
    }

    NODISCARD INLINE operator view_type() const noexcept { return view_type(begin(), end()); }

#ifdef _BASIC_STRING_H
    NODISCARD INLINE operator std::basic_string<Char>() const noexcept {
        return std::basic_string<Char>(data());
    }
#endif

    template <class _Tp>
    NODISCARD INLINE CONSTEXPR static enable_if_t<is_unsigned_v<_Tp>, self> from(_Tp __value) {
        auto __len = len(__value);
        self __str(__len);
        __to_chars(__str._impl + __len, __value);
        *(__str._impl + __len) = '\0';
        __str._size = __len;
        return __str;
    }

    template <class _Tp>
    NODISCARD INLINE CONSTEXPR static enable_if_t<is_signed_v<_Tp>, self> from(_Tp __value) {
        using _Up = typename unsigned_traits<_Tp>::type;

        _Up __val{_Up(__value)};
        size_t __len{0u};
        if (__value < 0) {
            __val = _Up(~__value) + _Up(1);
            __len = 1u;
        }
        __len += len(__val);

        self __str(__len);

        *__str._impl = '-';
        __to_chars<_Up>(__str._impl + __len, __val);
        *(__str._impl + __len) = '\0';
        __str._size = __len;

        return __str;
    }

    template <class _Tp, size_t Precision = 6u>
    NODISCARD INLINE CONSTEXPR static enable_if_t<is_floating_point_v<_Tp>, self> from(
        _Tp __value) {
        throw_if(__builtin_isnan(__value) || __builtin_isinf(__value));

        size_t __len{0u};
        _Tp __val{__value};
        if (__value < 0) {
            __val = -__value;
            __len = 1u;
        }

        auto __val_int = uint64_t(__val);
        __len += len(__val_int);

        self __str(__len + 1u + Precision);

        *__str._impl = '-';
        __to_chars<uint64_t>(__str._impl + __len, __val_int);
        auto __it = __str._impl + __len;

        auto __value_frac = __val - __val_int;
        *__it++ = '.';
        for (auto i = 0u; i < Precision - 1; ++i) {
            __value_frac *= 10u;
            auto digit = uint8_t(__value_frac);
            *__it++ = Char(digit + '0');
            __value_frac -= digit;
        }
        __value_frac *= 10u;
        auto digit = uint8_t(__value_frac);
        __value_frac -= digit;
        if (__value_frac > 0.5) {
            digit += 1;
        }
        *__it++ = Char(digit + '0');
        *(__str._impl + __len + 1u + Precision) = '\0';
        __str._size = __len + 1u + Precision;
        return __str;
    }
};

#ifdef _GLIBCXX_OSTREAM
template <class Char>
INLINE std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& out, view<Char> s) {
    out.write(s.data(), s.size());
    return out;
}

template <class Char, class Allocator>
INLINE std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& out,
                                            value<Char, Allocator> const& s) {
    out.write(s.data(), s.size());
    return out;
}
#endif

};  // namespace string

using string_view_t = string::view<char>;
using string_t = string::value<char>;

};  // namespace qlib
