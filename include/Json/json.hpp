#pragma once

#define JSON_IMPLEMENTATION

#include "Json/memory.h"
#include "Json/string.h"
#include "Json/vector.h"

namespace qlib {
namespace json {

enum class memory_policy { copy, view };

template <class T>
constexpr static bool_t is_number_v = is_signed_v<T> || is_unsigned_v<T> || is_floating_point_v<T>;

enum class error : int32_t {
    unknown = -1,
    impl_nullptr = -2,
    param_invalid = -3,
    file_not_found = -4,
    file_not_support = -5,
    file_invalid = -6,
    missing_left_brace = -7,
    missing_right_brace = -8,
    missing_left_quote = -9,
    missing_right_quote = -10,
    missing_colon = -11,
    missing_comma = -12,
    invalid_unicode = -13,
    invalid_null = -14,
    invalid_boolean = -15,
};

enum class value_enum : uint8_t {
    null = 0,
    object = 1 << 0,
    array = 1 << 1,
    string = 1 << 2,
    number = 1 << 3,
    boolean = 1 << 4,
    // string_ref = 1 << 5,
    number_ref = 1 << 6,
};

class not_object final : public exception {
public:
    char const* what() const noexcept override { return "not object"; }
};

class not_array final : public exception {
public:
    char const* what() const noexcept override { return "not array"; }
};

class not_string final : public exception {
public:
    char const* what() const noexcept override { return "not string"; }
};

class not_number final : public exception {
public:
    char const* what() const noexcept override { return "not number"; }
};

class not_boolean final : public exception {
public:
    char const* what() const noexcept override { return "not boolean"; }
};

template <class T, class Iter1, class Iter2>
INLINE CONSTEXPR auto exp(T& value, Iter1& first, Iter2 last) {
    if (*first == 'e' || *first == 'E') {
        ++first;

        T sign{1u};
        if (*first == '-') {
            sign = -1;
            ++first;
        } else if (*first == '+') {
            ++first;
        }

        T _exp{0u};
        while (likely(first < last && is_digit(*first))) {
            _exp = _exp * 10u + static_cast<T>(*first - '0');
            ++first;
        }

        value *= (T)__builtin_powi(10, sign * _exp);
    }
}

template <class T, class Enable = void>
class converter;

template <class T>
class converter<T, enable_if_t<is_unsigned_v<T>>> : public object {
public:
    template <class Iter1, class Iter2>
    INLINE CONSTEXPR static auto decode(T& __value, Iter1 __first, Iter2 __last) {
        T _value{0u};
        while (__first != __last && is_digit(*__first)) {
            _value = _value * 10u + (*__first - '0');
            ++__first;
        }

        exp(_value, __first, __last);

        throw_if(__first != __last, not_number());

        __value = _value;
    }
};

template <class T>
class converter<T, enable_if_t<is_signed_v<T>>> : public object {
public:
    template <class Iter1, class Iter2>
    INLINE CONSTEXPR static auto decode(T& value, Iter1 first, Iter2 last) {
        int8_t sign{1u};
        if (*first == '-') {
            sign = -1;
            ++first;
        }

        typename unsigned_traits<T>::type _value;
        converter<decltype(_value)>::decode(_value, first, last);

        value = sign * _value;
    }
};

template <class T>
class converter<T, enable_if_t<is_floating_point_v<T>>> : public object {
public:
    template <class Iter1, class Iter2>
    INLINE CONSTEXPR static auto decode(T& value, Iter1 first, Iter2 last) {
        T _value{0u};

        T sign = 1;
        if (*first == '-') {
            sign = -1;
            ++first;
        }

        while (first < last && is_digit(*first)) {
            _value = _value * 10u + static_cast<T>(*first - '0');
            ++first;
        }

        if (*first == '.') {
            ++first;
            T base = 0.1;
            while (first < last && is_digit(*first)) {
                _value += static_cast<T>(*first - '0') * base;
                base *= 0.1;
                ++first;
            }
        }

        exp(_value, first, last);

        throw_if(first != last, not_number());

        value = sign * _value;
    }
};

template <class... Args>
struct storage final {
    template <class... Ts>
    static constexpr size_t max_of() {
        size_t sizes[] = {sizeof(Ts)...};
        size_t max_size = sizes[0];
        for (size_t i = 1; i < sizeof...(Ts); ++i) {
            if (sizes[i] > max_size)
                max_size = sizes[i];
        }
        return max_size;
    }

    template <class... Ts>
    static constexpr size_t max_align_of() {
        size_t aligns[] = {alignof(Ts)...};
        size_t max_align = aligns[0];
        for (size_t i = 1; i < sizeof...(Ts); ++i) {
            if (aligns[i] > max_align)
                max_align = aligns[i];
        }
        return max_align;
    }

    static constexpr auto max_size = max_of<Args...>();
    static constexpr auto max_align = max_align_of<Args...>();

    union {
        uint8_t _value[max_size];
        struct __attribute__((__aligned__(max_align))) {
        } __align;
    };
};

template <class Json>
class parser;

template <class Char, memory_policy Policy, class Allocator = new_allocator_t>
class value final : public Allocator::reference {
public:
    using base = typename Allocator::reference;
    using self = value;
    using char_type = Char;
    using allocator_type = Allocator;
    using string_view_t = string::view<Char>;
    using string_t = string::value<Char, Allocator>;
    using key_type = conditional_t<Policy == memory_policy::view, string_view_t, string_t>;
    struct pair final {
        key_type key;
        self value;

        template <class Key, class Value>
        pair(Key&& _key, Value&& _value) : key(forward<Key>(_key)), value(forward<Value>(_value)) {}

        pair() = default;
        pair(pair const&) = default;
        pair(pair&&) = default;
        pair& operator=(pair const&) = default;
        pair& operator=(pair&&) = default;
    };
    using object_type = vector_t<pair, Allocator>;
    using array_type = vector_t<self, Allocator>;
    using string_type = conditional_t<Policy == memory_policy::view, string_view_t, string_t>;
    using size_type = uint32_t;
    const static self default_value;

protected:
    value_enum _type{value_enum::null};
    using impl_type = storage<object_type, array_type, string_t, string_view_t>;
    impl_type _impl{};

    friend class parser<self>;

    constexpr static string_view_t true_str = "true";
    constexpr static string_view_t false_str = "false";

    struct FixedOutStream final : public Allocator::reference {
    protected:
        Char* _impl{nullptr};
        size_type _size{0u};
        size_type _capacity{0u};

        NODISCARD INLINE allocator_type& _allocator() noexcept {
            using reference = typename allocator_type::reference;
            return static_cast<reference&>(*this);
        }
        NODISCARD INLINE allocator_type const& _allocator() const noexcept {
            using reference = typename allocator_type::reference;
            return static_cast<reference&>(*this);
        }

    public:
        using base = typename Allocator::reference;

        INLINE constexpr explicit FixedOutStream(size_type capacity) noexcept(
            is_nothrow_constructible_v<base>)
                : _impl(_allocator().template allocate<Char>(capacity + 1u)), _size(0u),
                  _capacity(capacity) {}

        INLINE constexpr FixedOutStream(size_type capacity, Allocator& allocator) noexcept(
            is_nothrow_constructible_v<base, Allocator&>)
                : base(allocator), _impl(_allocator().template allocate<Char>(capacity + 1u)),
                  _size(0u), _capacity(capacity) {}

        FixedOutStream(FixedOutStream const&) = delete;
        FixedOutStream& operator=(FixedOutStream const&) = delete;
        INLINE FixedOutStream(FixedOutStream&& o)
                : base(move(o)), _impl(o._impl), _size(o._size), _capacity(o._capacity) {
            o._impl = nullptr;
            o._size = 0u;
            o._capacity = 0u;
        }

        INLINE FixedOutStream& operator=(FixedOutStream&& o) noexcept {
            this->~FixedOutStream();
            new (this) FixedOutStream(move(o));
            return *this;
        }

        INLINE ~FixedOutStream() {
            _allocator().template deallocate<Char>(_impl, _capacity);
            _impl = nullptr;
            _size = 0u;
            _capacity = 0u;
        }

        INLINE FixedOutStream& operator<<(string_view_t s) {
            size_type new_size = _size + s.size();
            throw_if(new_size > _capacity);
            copy(s.begin(), s.end(), _impl + _size);
            _size = new_size;
            return *this;
        }

        NODISCARD INLINE bool_t operator==(string_view_t s) const {
            return _size == s.size() && equal(s.begin(), s.end(), _impl);
        }
    };

    NODISCARD INLINE allocator_type& _allocator() noexcept {
        using reference = typename allocator_type::reference;
        return static_cast<reference&>(*this);
    }
    NODISCARD INLINE allocator_type& _allocator() const noexcept {
        using reference = typename allocator_type::reference;
        return const_cast<reference&>(static_cast<reference const&>(*this));
    }

    template <class Iter1, class Iter2>
    INLINE static constexpr int32_t _parse_unicode(uint32_t& code, Iter1& begin, Iter2) {
        int32_t result{0u};
        do {
            code = 0;
            for (uint8_t i = 0; i < 4; ++i) {
                Char c = *begin;
                code <<= 4;
                if (c >= '0' && c <= '9') {
                    code |= (c - '0');
                } else if (c >= 'A' && c <= 'F') {
                    code |= (c - 'A' + 10);
                } else if (c >= 'a' && c <= 'f') {
                    code |= (c - 'a' + 10);
                } else {
                    result = -1;
                    break;
                }
                ++begin;
            }
            if (unlikely(!result && code > (0xD800 - 1) && code < 0xDC00)) {
                if (*begin != '\\' || *(begin + 1) != 'u') {
                    result = -1;
                    break;
                }
                begin += 2;  // 跳过 \u
                uint16_t code2{0u};
                for (uint8_t i = 0; i < 4; ++i) {
                    Char c = *begin;
                    code2 <<= 4;
                    if (c >= '0' && c <= '9') {
                        code2 |= (c - '0');
                    } else if (c >= 'A' && c <= 'F') {
                        code2 |= (c - 'A' + 10);
                    } else if (c >= 'a' && c <= 'f') {
                        code2 |= (c - 'a' + 10);
                    } else {
                        result = -1;
                        break;
                    }
                    ++begin;
                }
                code = 0x10000 + ((code - 0xD800) << 10) + (code2 - 0xDC00);
            }
        } while (0);
        return result;
    }

    template <class Iter1, class Iter2>
    INLINE static constexpr int32_t _parse_string(string_t* value, Iter1 begin, Iter2 end) {
        int32_t result{0u};

        auto start = begin;
        while (begin < end && result == 0) {
            if (*begin != '\\') {
                ++begin;
            } else {
                *value << string_view_t{start, begin};
                ++begin;
                switch (*begin) {
                    case '"':
                    case '\\':
                    case '/':
                        *value << *begin;
                        ++begin;
                        break;
                    case 'b':
                        *value << '\b';
                        ++begin;
                        break;
                    case 'f':
                        *value << '\f';
                        ++begin;
                        break;
                    case 'n':
                        *value << '\n';
                        ++begin;
                        break;
                    case 'r':
                        *value << '\r';
                        ++begin;
                        break;
                    case 't':
                        *value << '\t';
                        ++begin;
                        break;
                    case 'u': {
                        ++begin;
                        uint32_t code{0u};
                        result = _parse_unicode(code, begin, end);
                        if (0 != result) {
                            break;
                        }
                        if (code <= 0x7f) {
                            *value << (char)code;
                        } else if (code <= 0x7ff) {
                            *value << (Char)(0xc0 | (code >> 6));
                            *value << (Char)(0x80 | (code & 0x3f));
                        } else if (code <= 0xffff) {
                            *value << (Char)(0xe0 | (code >> 12));
                            *value << (Char)(0x80 | ((code >> 6) & 0x3f));
                            *value << (Char)(0x80 | (code & 0x3f));
                        } else if (code <= 0x10ffff) {
                            *value << (Char)(0xf0 | (code >> 18));
                            *value << (Char)(0x80 | ((code >> 12) & 0x3f));
                            *value << (Char)(0x80 | ((code >> 6) & 0x3f));
                            *value << (Char)(0x80 | (code & 0x3f));
                        } else {
                            result = -1;
                        }
                        break;
                    }
                    default: {
                        *value << *begin;
                        ++begin;
                    }
                }
                start = begin;
            }
        }
        if (!result) {
            *value << string_view_t{start, begin};
        }
        return result;
    }

    template <class T = string_type>
    INLINE constexpr enable_if_t<is_same_v<T, string_view_t>, void> _init_string_type(
        string_view_t value, allocator_type&) {
        new (&_impl) string_type(value);
    }

    template <class T = string_type>
    INLINE constexpr enable_if_t<!is_same_v<T, string_view_t>, void> _init_string_type(
        string_view_t value, allocator_type& allocator) {
        new (&_impl) string_type(value, allocator);
    }

    template <class T = string_type>
    INLINE constexpr enable_if_t<is_same_v<T, string_view_t>, void> _object_emplace(
        object_type& object, string_view_t key) {
        object.emplace_back(key, self(_allocator()));
    }

    template <class T = string_type>
    INLINE constexpr enable_if_t<!is_same_v<T, string_view_t>, void> _object_emplace(
        object_type& object, string_view_t key) {
        object.emplace_back(string_t(key, _allocator()), self(_allocator()));
    }

public:
    template <class T>
    class value_ref final : public object {
    public:
        using self = value_ref;
        using value_type = T;

    protected:
        mutable T _impl;

    public:
        value_ref() = delete;
        value_ref(self const&) = delete;
        INLINE value_ref(self&&) = default;
        self& operator=(self const&) = delete;
        INLINE self& operator=(self&&) = default;

        template <class... Args>
        INLINE value_ref(Args&&... args) : _impl(forward<Args>(args)...) {}

        INLINE value_type operator*() const noexcept { return move(_impl); }
    };

    // template <class Enable = enable_if_t<is_constructible_v<base>>>
    INLINE constexpr value() noexcept(is_nothrow_constructible_v<base>){};

    INLINE constexpr value(allocator_type& allocator) noexcept(
        is_nothrow_constructible_v<base, allocator_type&>)
            : base(allocator) {}

#define REGISTER_CONSTRUCTOR(enum_value, type)                                                     \
    INLINE constexpr value(type const& value) : _type(enum_value) { new (&_impl) type(value); }    \
    INLINE constexpr value(type const& value, allocator_type& allocator)                           \
            : base(allocator), _type(enum_value) {                                                 \
        new (&_impl) type(value);                                                                  \
    }                                                                                              \
    INLINE constexpr value(type&& value) : _type(enum_value) { new (&_impl) type(move(value)); }   \
    INLINE constexpr value(type&& value, allocator_type& allocator)                                \
            : base(allocator), _type(enum_value) {                                                 \
        new (&_impl) type(move(value));                                                            \
    }

    REGISTER_CONSTRUCTOR(value_enum::object, object_type)
    REGISTER_CONSTRUCTOR(value_enum::array, array_type)
    // REGISTER_CONSTRUCTOR(value_enum::string, string_type)

#undef REGISTER_CONSTRUCTOR

    constexpr value(Char const* value) : _type(value_enum::string) {
        new (&_impl) string_type(value);
    }

    INLINE constexpr value(string_view_t value, allocator_type& allocator)
            : base(allocator), _type(value_enum::string) {
        _init_string_type(value, allocator);
    }

    INLINE constexpr value(string_view_t value) : _type(value_enum::string) {
        new (&_impl) string_type(value);
    }

    INLINE constexpr value(string_t const& value) : _type(value_enum::string) {
        new (&_impl) string_type(value);
    }

    INLINE constexpr value(string_t const& value, allocator_type& allocator)
            : base(allocator), _type(value_enum::string) {
        new (&_impl) string_type(value);
    }

    INLINE constexpr value(string_t&& value) : _type(value_enum::string) {
        new (&_impl) string_type(move(value));
    }

    INLINE constexpr value(string_t&& value, allocator_type& allocator)
            : base(allocator), _type(value_enum::string) {
        new (&_impl) string_type(move(value));
    }

    template <class T, class Enable = enable_if_t<is_number_v<T>>>
    constexpr value(T value) : _type(value_enum::number) {
        new (&_impl) string_t(string_t::from(value));
    }

    // template <class Enable = enable_if_t<is_constructible_v<base>>>
    constexpr value(bool_t value) : _type(value_enum::boolean) {
        new (&_impl) string_view_t(value ? true_str : false_str);
    }

    constexpr value(bool_t value, allocator_type& allocator)
            : base(allocator), _type(value_enum::boolean) {
        new (&_impl) string_view_t(value ? true_str : false_str);
    }

    constexpr value(self const& o) : base(o), _type(o._type) {
        switch (_type) {
            case value_enum::object: {
                new (&_impl) object_type(*(object_type*)(&o._impl));
                break;
            }
            case value_enum::array: {
                new (&_impl) array_type(*(array_type*)(&o._impl));
                break;
            }
            case value_enum::string: {
                new (&_impl) string_type(*(string_type*)(&o._impl));
                break;
            }
            case value_enum::number: {
                new (&_impl) string_t(*(string_t*)(&o._impl));
                break;
            }
            case value_enum::boolean: {
                new (&_impl) string_view_t(*(string_view_t*)(&o._impl));
                break;
            }
            case value_enum::number_ref: {
                new (&_impl) string_type(*(string_type*)(&o._impl));
                break;
            }
            default:;
        }
    }

    INLINE CONSTEXPR value(self&& o) : base(move(o)), _type{o._type}, _impl{move(o._impl)} {
        o._type = value_enum::null;
    }

#ifdef _INITIALIZER_LIST
    constexpr value(std::initializer_list<value_ref<pair>> list) : _type(value_enum::object) {
        new (&_impl) object_type(list.size());
        auto object = (object_type*)(&_impl);
        for (auto const& item : list) {
            object->emplace_back(move(*item));
        }
    }
#endif

    ~value() noexcept {
        switch (_type) {
            case value_enum::object: {
                ((object_type*)&_impl)->~object_type();
                break;
            }
            case value_enum::array: {
                ((array_type*)&_impl)->~array_type();
                break;
            }
            case value_enum::string: {
                ((string_type*)&_impl)->~string_type();
                break;
            }
            case value_enum::number: {
                ((string_t*)&_impl)->~string_t();
                break;
            }
            case value_enum::boolean: {
                ((string_view_t*)&_impl)->~string_view_t();
                break;
            }
            case value_enum::number_ref: {
                ((string_type*)&_impl)->~string_type();
                break;
            }
            default:;
        }
    }

    template <class T>
    INLINE self& operator=(T&& o) {
        this->~value();
        new (this) self(forward<T>(o), _allocator());
        return *this;
    }

    INLINE self& operator=(self const& o) {
        if (unlikely(this != &o)) {
            this->~value();
            new (this) self(o);
        }
        return *this;
    }

    INLINE self& operator=(self&& o) {
        this->~value();
        new (this) self(move(o));
        return *this;
    }

    NODISCARD INLINE constexpr bool_t empty() const noexcept { return _type == value_enum::null; }

    NODISCARD INLINE constexpr auto type() const noexcept { return _type; }

    template <class T>
    NODISCARD INLINE constexpr enable_if_t<is_number_v<T>, T> get() const {
        if (likely(_type == value_enum::number_ref)) {
            auto& value = *(string_type*)(&_impl);
            T value_number{};
            converter<T>::decode(value_number, value.begin(), value.end());
            return value_number;
            // return (*(string_type*)(&_impl)).template to<T>();
        } else if (likely(_type == value_enum::number)) {
            auto& value = *(string_t*)(&_impl);
            T value_number{};
            converter<T>::decode(value_number, value.begin(), value.end());
            return value_number;
            // return (*(string_t*)(&_impl)).template to<T>();
        } else {
            __throw(not_number{});
        }
    }

    template <class T>
    NODISCARD INLINE constexpr enable_if_t<is_same_v<T, bool_t>, T> get() const {
        if (likely(_type == value_enum::boolean)) {
            auto& value = *(string_view_t*)(&_impl);
            if (value == true_str) {
                return True;
            } else if (value == false_str) {
                return False;
            } else {
                __throw(not_boolean{});
            }
        } else {
            __throw(not_boolean{});
        }
    }

    template <class T>
    NODISCARD INLINE constexpr enable_if_t<is_same_v<T, string_view_t>, T> get() const {
        if (_type == value_enum::string) {
            return *(string_type*)(&_impl);
        } else {
            __throw(not_string{});
        }
    }

    template <class T>
    NODISCARD INLINE constexpr enable_if_t<is_same_v<T, string_t>, T> get() const {
        if (_type == value_enum::string) {
            auto& s = *(string_type*)(&_impl);
            string_t result(s.size(), _allocator());
            if (unlikely(_parse_string(&result, s.begin(), s.end()))) {
                __throw(not_string{});
            }
            return result;
        } else {
            __throw(not_string{});
        }
    }

    template <class T>
    NODISCARD INLINE constexpr T get(T&& default_value) const {
        if (empty()) {
            return forward<T>(default_value);
        }
        return get<T>();
    }

    NODISCARD INLINE constexpr self const& operator[](string_view_t key) const {
        for (auto const& pair : object()) {
            if (pair.key == key) {
                return pair.value;
            }
        }
        return default_value;
    }

    NODISCARD INLINE constexpr self& operator[](string_view_t key) {
        auto& object = this->object();
        for (auto& pair : object) {
            if (pair.key == key) {
                return pair.value;
            }
        }
        _object_emplace(object, key);
        return object.back().value;
    }

    NODISCARD FORCE_INLINE object_type& object() {
        throw_if(_type != value_enum::object, not_object{});
        return *(object_type*)(&_impl);
    }
    NODISCARD FORCE_INLINE object_type const& object() const {
        return const_cast<self&>(*this).object();
    }

    NODISCARD FORCE_INLINE array_type& array() {
        throw_if(_type != value_enum::array, not_array{});
        return *(array_type*)(&_impl);
    }

    NODISCARD FORCE_INLINE array_type const& array() const {
        return const_cast<self&>(*this).array();
    }

#ifdef _INITIALIZER_LIST
    NODISCARD INLINE static self object(std::initializer_list<value_ref<pair>> list) {
        self value;
        value._type = value_enum::object;
        new (&value._impl) object_type(list.size());
        auto object = (object_type*)(&value._impl);
        for (auto const& item : list) {
            object->emplace_back(move(*item));
        }
        return value;
    }

    NODISCARD INLINE static self array(std::initializer_list<value_ref<self>> list) {
        self value;
        value._type = value_enum::array;
        new (&value._impl) array_type(list.size());
        auto array = (array_type*)(&value._impl);
        for (auto const& item : list) {
            array->emplace_back(move(*item));
        }
        return value;
    }
#endif

    template <class OutStream>
    constexpr OutStream& to(OutStream& out) const {
        constexpr string_view_t null_str{"null"};
        constexpr string_view_t quote_str{"\""};
        constexpr string_view_t comma_str{","};
        constexpr string_view_t colon_str{":"};

        switch (_type) {
            case value_enum::null: {
                out << null_str;
                break;
            }
            case value_enum::string: {
                out << quote_str << *(string_type*)(&_impl) << quote_str;
                break;
            }
            case value_enum::number: {
                out << *((string_t*)(&_impl));
                break;
            }
            case value_enum::number_ref: {
                out << *((string_type*)(&_impl));
                break;
            }
            case value_enum::boolean: {
                out << *((string_view_t*)(&_impl));
                break;
            }
            case value_enum::array: {
                out << "[";
                auto& array = this->array();
                for (auto it = array.begin(); it != array.end();) {
                    out << *it;
                    ++it;
                    if (likely(it != array.end())) {
                        out << comma_str;
                    }
                }
                out << "]";
                break;
            }
            case value_enum::object: {
                out << "{";
                auto& object = this->object();
                for (auto it = object.begin(); it != object.end();) {
                    out << quote_str << it->key << quote_str << colon_str << it->value;
                    ++it;
                    if (likely(it != object.end())) {
                        out << comma_str;
                    }
                }
                out << "}";
                break;
            }
            default:;
        }
        return out;
    }

    NODISCARD INLINE constexpr auto to() const {
        string_t out(1024u);
        return to(out);
    }

    NODISCARD INLINE explicit operator bool_t() const noexcept { return !empty(); }

    NODISCARD INLINE bool_t operator==(string_view_t text) const {
        FixedOutStream out(text.size());
        bool_t ok{False};
        try {
            ok = (to(out) == text);
        } catch (exception const& _) {
            ok = False;
        }
        return ok;
    }

    template <class T>
    NODISCARD INLINE bool_t operator!=(T const& o) const {
        return !(*this == o);
    }
};

template <class Char, memory_policy Policy, class Allocator>
const typename value<Char, Policy, Allocator>::self value<Char, Policy, Allocator>::default_value =
    value<Char, Policy, Allocator>{};

// #if __cplusplus < 201703L
// template <class Char, memory_policy Policy, class Allocator>
// constexpr typename value<Char, Policy, Allocator>::string_view_t
//     value<Char, Policy, Allocator>::true_str = "true";

// template <class Char, memory_policy Policy, class Allocator>
// constexpr typename value<Char, Policy, Allocator>::string_view_t
//     value<Char, Policy, Allocator>::false_str = "false";
// #endif

template <class Char, memory_policy Policy, class Allocator>
constexpr
    typename value<Char, Policy, Allocator>::string_view_t value<Char, Policy, Allocator>::true_str;

template <class Char, memory_policy Policy, class Allocator>
constexpr typename value<Char, Policy, Allocator>::string_view_t
    value<Char, Policy, Allocator>::false_str;

template <class Json>
class parser final : public object {
public:
    using size_type = size_t;
    using json_type = Json;
    using allocator_type = typename Json::allocator_type;
    using string_type = typename json_type::string_type;
    using array_type = typename json_type::array_type;
    using object_type = typename json_type::object_type;
    using key_type = typename json_type::key_type;
    using string_t = typename json_type::string_t;
    using string_view_t = typename json_type::string_view_t;

protected:
    size_type _capacity{16u};

    struct impl {
        bool_t is_object;
        void* _impl{nullptr};

        impl(object_type* object) : is_object{True}, _impl{object} {}
        impl(array_type* array) : is_object{False}, _impl{array} {}

        object_type* object() { return (object_type*)(_impl); }
        array_type* array() { return (array_type*)(_impl); }
    };
    using impl_type = impl;

    template <class T = key_type>
    INLINE static enable_if_t<is_same_v<T, string_view_t>, json_type> create_number_ref(
        string_view_t value, allocator_type& allocator) {
        json_type json_value(allocator);
        json_value._type = value_enum::number_ref;
        new (&json_value._impl) string_type(value);
        return json_value;
    }

    template <class T = key_type>
    INLINE static enable_if_t<!is_same_v<T, string_view_t>, json_type> create_number_ref(
        string_view_t value, allocator_type& allocator) {
        json_type json_value(allocator);
        json_value._type = value_enum::number_ref;
        new (&json_value._impl) string_type(value, allocator);
        return json_value;
    }

    template <class T = key_type>
    FORCE_INLINE enable_if_t<is_same_v<T, string_view_t>> __object_emplace(impl& layer,
                                                                           string_view_t key,
                                                                           json_type&& value) {
        layer.object()->emplace_back(key, move(value));
    }

    template <class T = key_type>
    FORCE_INLINE enable_if_t<!is_same_v<T, string_view_t>> __object_emplace(impl& layer,
                                                                            string_view_t key,
                                                                            json_type&& value) {
        layer.object()->emplace_back(string_type(key, value._allocator()), move(value));
    }

    class char_helper final {
    protected:
        uint8_t _impl[256u];

    public:
        enum class type : uint8_t {
            skip = 1u,
            quote,
            left_brace,
            right_brace,
            left_bracket,
            right_bracket,
            n,
            t,
            f,
            number,
        };
        CONSTEXPR INLINE char_helper() noexcept : _impl{} {
            string_view_t __skip{" \t\n\r:,"};
            for (auto it = __skip.begin(); it != __skip.end(); ++it) {
                _impl[uint8_t(*it)] = 1u;
            }

            _impl[uint8_t('"')] = 2u;

            _impl[uint8_t('{')] = 3u;
            _impl[uint8_t('}')] = 4u;

            _impl[uint8_t('[')] = 5u;
            _impl[uint8_t(']')] = 6u;

            _impl[uint8_t('n')] = 7u;
            _impl[uint8_t('t')] = 8u;
            _impl[uint8_t('f')] = 9u;

            string_view_t __number{"0123456789eE+-."};
            for (auto it = __number.begin(); it != __number.end(); ++it) {
                _impl[uint8_t(*it)] = 10u;
            }
        }

        NODISCARD FORCE_INLINE CONSTEXPR type operator[](uint8_t c) const noexcept {
            return type(_impl[c]);
        }
    };

#if __cplusplus >= 201703L
    CONSTEXPR static char_helper _char_helper{};
#else  // C++14
    CONSTEXPR static char_helper _char_helper;
#endif

    template <class Iter1, class Iter2>
    FORCE_INLINE Iter1 _parse_string(string_view_t* value, Iter1 begin, Iter2 end) {
        Iter1 start = ++begin;
        while (begin < end && *begin != '"') {
            if (*begin == '\\' && ++begin < end) {
                if (*begin == 'u') {
                    begin += 4;
                } else {
                    ++begin;
                }
            } else {
                ++begin;
            }
        }

        *value = string_view_t(start, begin);
        return ++begin;
    }

    template <class Iter1, class Iter2>
    FORCE_INLINE CONSTEXPR int32_t
    _call(json_type* json, Iter1 begin, Iter2 end, vector_t<impl_type>& layers) {
        int32_t result{0};

        do {
            while (begin < end && _char_helper[uint8_t(*begin)] == char_helper::type::skip) {
                ++begin;
            }

            auto& allocator = json->_allocator();
            json_type root(allocator);

            if (*begin == '{') {
                root._type = value_enum::object;
                new (&root._impl) object_type(_capacity, allocator);
                layers.emplace_back(&root.object());
            } else if (*begin == '[') {
                root._type = value_enum::array;
                new (&root._impl) array_type(_capacity, allocator);
                layers.emplace_back(&root.array());
            } else {
                result = int32_t(error::missing_left_brace);
                break;
            }
            ++begin;

            while (begin < end && !result) {
                while (begin < end && _char_helper[uint8_t(*begin)] == char_helper::type::skip) {
                    ++begin;
                }

                auto& last_layer = layers.back();
                if (last_layer.is_object) {
                    if (*begin == '}') {
                        layers.pop_back();
                        ++begin;
                        if (layers.empty()) {
                            result = 1;
                            break;
                        }
                        continue;
                    }
                    string_view_t key;
                    begin = _parse_string(&key, begin, end);
                    while (begin < end &&
                           _char_helper[uint8_t(*begin)] == char_helper::type::skip) {
                        ++begin;
                    }
                    switch (_char_helper[uint8_t(*begin)]) {
                        case char_helper::type::quote: {
                            string_view_t value;
                            begin = _parse_string(&value, begin, end);
                            __object_emplace(last_layer, key, json_type(value, allocator));
                            break;
                        }
                        case char_helper::type::left_brace: {
                            json_type value(allocator);
                            value._type = value_enum::object;
                            new (&value._impl) object_type(_capacity, allocator);
                            __object_emplace(last_layer, key, move(value));
                            layers.emplace_back(&last_layer.object()->back().value.object());
                            ++begin;
                            break;
                        }
                        case char_helper::type::left_bracket: {
                            json_type value(allocator);
                            value._type = value_enum::array;
                            new (&value._impl) array_type(_capacity, allocator);
                            __object_emplace(last_layer, key, move(value));
                            layers.emplace_back(&last_layer.object()->back().value.array());
                            ++begin;
                            break;
                        }
                        case char_helper::type::n: {
                            if (likely(*(begin + 1) == 'u' && *(begin + 2) == 'l' &&
                                       *(begin + 3) == 'l')) {
                                __object_emplace(last_layer, key, json_type(allocator));
                            } else {
                                result = (int32_t)error::invalid_null;
                            }
                            begin += 4;
                            break;
                        }
                        case char_helper::type::t: {
                            if (likely(*(begin + 1) == 'r' && *(begin + 2) == 'u' &&
                                       *(begin + 3) == 'e')) {
                                __object_emplace(last_layer, key, json_type(true, allocator));
                            } else {
                                result = (int32_t)error::invalid_boolean;
                            }
                            begin += 4;
                            break;
                        }
                        case char_helper::type::f: {
                            if (likely(begin[1] == 'a' && begin[2] == 'l' && begin[3] == 's' &&
                                       begin[4] == 'e')) {
                                __object_emplace(last_layer, key, json_type(false, allocator));
                            } else {
                                result = (int32_t)error::invalid_boolean;
                            }
                            begin += 5;
                            break;
                        }
                        default: {
                            auto start = begin;
                            while (begin < end &&
                                   _char_helper[(uint8_t)*begin] == char_helper::type::number) {
                                ++begin;
                            }
                            auto stop = begin;
                            if (likely(stop > start)) {
                                __object_emplace(
                                    last_layer, key,
                                    create_number_ref(string_view_t{start, stop}, allocator));
                            }
                        }
                    }
                } else {
                    switch (_char_helper[uint8_t(*begin)]) {
                        case char_helper::type::quote: {
                            string_view_t value;
                            begin = _parse_string(&value, begin, end);
                            last_layer.array()->emplace_back(value, allocator);
                            break;
                        }
                        case char_helper::type::left_bracket: {
                            json_type value(allocator);
                            value._type = value_enum::array;
                            new (&value._impl) array_type(_capacity, allocator);
                            last_layer.array()->emplace_back(move(value));
                            layers.emplace_back(&last_layer.array()->back().array());
                            ++begin;
                            break;
                        }
                        case char_helper::type::right_bracket: {
                            layers.pop_back();
                            ++begin;
                            if (layers.empty()) {
                                result = 1;
                            }
                            break;
                        }
                        case char_helper::type::left_brace: {
                            json_type value(allocator);
                            value._type = value_enum::object;
                            new (&value._impl) object_type(_capacity, allocator);
                            last_layer.array()->emplace_back(move(value));
                            layers.emplace_back(&last_layer.array()->back().object());
                            ++begin;
                            continue;
                        }
                        case char_helper::type::n: {
                            if (likely(*(begin + 1) == 'u' && *(begin + 2) == 'l' &&
                                       *(begin + 3) == 'l')) {
                                last_layer.array()->emplace_back(allocator);
                            } else {
                                result = (int32_t)error::invalid_null;
                            }
                            begin += 4;
                            break;
                        }
                        case char_helper::type::t: {
                            if (likely(*(begin + 1) == 'r' && *(begin + 2) == 'u' &&
                                       *(begin + 3) == 'e')) {
                                last_layer.array()->emplace_back(true, allocator);
                            } else {
                                result = (int32_t)error::invalid_boolean;
                            }
                            begin += 4;
                            break;
                        }
                        case char_helper::type::f: {
                            if (likely(begin[1] == 'a' && begin[2] == 'l' && begin[3] == 's' &&
                                       begin[4] == 'e')) {
                                last_layer.array()->emplace_back(false, allocator);
                            } else {
                                result = (int32_t)error::invalid_boolean;
                            }
                            begin += 5;
                            break;
                        }
                        default: {
                            auto start = begin;
                            while (begin < end &&
                                   _char_helper[(uint8_t)*begin] == char_helper::type::number) {
                                ++begin;
                            }
                            auto stop = begin;
                            if (likely(stop > start)) {
                                last_layer.array()->emplace_back(
                                    create_number_ref(string_view_t{start, stop}, allocator));
                            }
                        }
                    }
                }
            }

            if (result == 1) {
                *json = move(root);
                result = 0;
            }
        } while (false);

        return result;
    }

public:
    constexpr parser() noexcept = default;

    constexpr parser(size_type capacity) noexcept : _capacity(capacity) {}

    template <class Iter1, class Iter2>
    FORCE_INLINE CONSTEXPR int32_t operator()(json_type* json, Iter1 begin, Iter2 end) {
        vector_t<impl_type> layers(_capacity, json->_allocator());
        return _call(json, begin, end, layers);
    }
};

#if __cplusplus < 201703L
template <class Json>
typename parser<Json>::char_helper parser<Json>::_char_helper{};
#endif

template <class Iter1, class Iter2, class Json>
FORCE_INLINE CONSTEXPR int32_t parse(Json* json, Iter1 begin, Iter2 end) noexcept {
    parser<Json> parser;
    return parser(json, begin, end);
}

#ifdef _GLIBCXX_FSTREAM
template <class Char, memory_policy Policy, class Allocator>
INLINE constexpr int32_t parse(value<Char, Policy, Allocator>* ptr,
                               std::basic_ifstream<Char> const& stream) noexcept {
    int32_t result{0};

    do {
        if (unlikely(!stream.is_open())) {
            result = static_cast<int32_t>(error::file_not_found);
            break;
        }

        auto text =
            std::string{std::istreambuf_iterator<Char>(stream), std::istreambuf_iterator<Char>()};
        result = parse(ptr, text.data(), text.data() + text.size());
    } while (false);

    return result;
}

template <class Char, memory_policy Policy, class Allocator>
INLINE int32_t parse(value<Char, Policy, Allocator>* ptr, string_view_t file) noexcept {
    return parse(ptr, std::basic_ifstream<Char>{file});
}
#endif

template <class OutStream, class Char, memory_policy Policy, class Allocator>
INLINE OutStream& operator<<(OutStream& out, value<Char, Policy, Allocator> const& value) {
    value.to(out);
    return out;
}

};  // namespace json

using memory_policy_t = json::memory_policy;
using json_t = json::value<char, json::memory_policy::copy>;
using json_view_t = json::value<char, json::memory_policy::view>;
using json_pool_t = json::value<char, json::memory_policy::copy, pool_allocator_t>;
using json_view_pool_t = json::value<char, json::memory_policy::view, pool_allocator_t>;

};  // namespace qlib
