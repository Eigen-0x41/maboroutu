module;
#include <bit>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
export module maboroutu.core;

#ifdef __clang__
static_assert(__clang_major__ >= 18, "maboroutu requires Clang 18+");
#elifdef __GNUC__
static_assert(__GNUC__ >= 15, "maboroutu requires GCC 15+");
#elif defined(_MSC_VER)
static_assert(
    _MSC_VER >= 1944,
    "maboroutu requires MSVC Build Tools 14.44+ (Visual Studio 2022)");
#endif

namespace maboroutu {
export using std::endian;

export constexpr std::size_t recommended_alignment =
#ifdef __cpp_lib_hardware_interference_size
    std::hardware_destructive_interference_size
#else
    64
#endif
    ;

export template <typename T> struct basic_region {
   using value_type = T;

   value_type offset;
   value_type size;
};

export using region = basic_region<std::size_t>;

export template <typename T> struct dynamic_array {
   using element_type = T;
   using value_type = element_type[];
   std::unique_ptr<value_type> value;
   std::size_t size;
};

export using byte_array = dynamic_array<std::byte>;

export template <class T, class... ArgsT>
concept one_of = (std::same_as<T, ArgsT> || ...);

export template <class... Args>
struct contains_duplicate_t : std::false_type {};
template <class T, class... Args>
struct contains_duplicate_t<T, Args...>
    : std::conditional_t<one_of<T, Args...>, std::true_type,
                         contains_duplicate_t<Args...>> {};
export template <class... Args>
concept contains_duplicate = contains_duplicate_t<Args...>::value;

static_assert(not contains_duplicate<char>, "not duplicate");
static_assert(not contains_duplicate<char, short>, "not duplicate");
static_assert(not contains_duplicate<char, short, int>, "not duplicate");
static_assert(contains_duplicate<char, short, int, short>, "duplicate");
static_assert(contains_duplicate<int, short, int, short>, "duplicate");
static_assert(not contains_duplicate<char, short, int, int &>, "not duplicate");

export template <class EnumT, EnumT EnumV>
   requires std::is_enum_v<EnumT>
struct in_place_tag {
   using value_type = EnumT;
   static constexpr value_type value = EnumV;

   in_place_tag() = default;
   in_place_tag(in_place_tag const &) = default;
   in_place_tag(in_place_tag &&) = default;

   auto operator=(const in_place_tag &) -> in_place_tag & = delete;
   auto operator=(in_place_tag &&) -> in_place_tag & = delete;
};

export template <auto EnumV>
constexpr auto in_place_tag_v = in_place_tag<decltype(EnumV), EnumV>();

export template <endian EndianV>
using endian_in_place_tag = in_place_tag<endian, EndianV>;

export template <class C, class T>
struct copy_const
    : public std::conditional<std::is_const_v<C>, std::add_const_t<T>, T> {};
export template <class C, class T> using copy_const_t = copy_const<C, T>::type;

template <class T, class F>
concept read_lockable_with =
    requires(T &value, F &&func) { value.read_lock(std::forward<F>(func)); };
template <class T, class F>
concept write_lockable_with =
    requires(T &value, F &&func) { value.write_lock(std::forward<F>(func)); };

export template <class T, class F>
   requires read_lockable_with<T, F>
constexpr auto invoke_within_read_lock_or_directly(T &value, F &&func) noexcept(
    noexcept(value.read_lock(std::forward<F>(func))))
    -> decltype(value.read_lock(std::forward<F>(func))) {
   return value.read_lock(std::forward<F>(func));
}

export template <class T, class F>
   requires(!read_lockable_with<T, F>)
constexpr auto invoke_within_read_lock_or_directly(T &value, F &&func) noexcept(
    std::is_nothrow_invocable_v<F &&, T &>) -> std::invoke_result_t<F &&, T &> {
   return std::invoke(std::forward<F>(func), value);
}

export template <class T, class F>
   requires write_lockable_with<T, F>
constexpr auto invoke_within_write_lock_or_directly(
    T &value,
    F &&func) noexcept(noexcept(value.write_lock(std::forward<F>(func))))
    -> decltype(value.write_lock(std::forward<F>(func))) {
   return value.write_lock(std::forward<F>(func));
}

export template <class T, class F>
   requires(!write_lockable_with<T, F> && !std::is_const_v<T>)
constexpr auto invoke_within_write_lock_or_directly(
    T &value, F &&func) noexcept(std::is_nothrow_invocable_v<F &&, T &>)
    -> std::invoke_result_t<F &&, T &> {
   return std::invoke(std::forward<F>(func), value);
}

} // namespace maboroutu
