module;
#include <bit>
#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>
export module maboroutu.core;

#if defined(__clang__)
static_assert(__clang_major__ >= 16, "mylib requires Clang 16+");
#elif defined(__GNUC__)
static_assert(__GNUC__ >= 13, "mylib requires GCC 13+");
#elif defined(_MSC_VER)
static_assert(_MSC_VER >= 1951,
              "mylib requires MSVC Build Tools 14.51+ (Visual Studio 2026)");
#endif

namespace maboroutu {
export template <typename T> struct basic_region {
   T offset;
   T size;
};

export using region = basic_region<std::size_t>;

export template <typename T> struct dynamic_array {
   using value_type = T;
   std::unique_ptr<T[]> value;
   std::size_t size;
};

export using byte_array = dynamic_array<std::byte>;

export template <class T, class... ArgsT>
concept one_of = (std::same_as<T, ArgsT> || ...);

export template <class... Args>
struct contains_duplicate_t : std::false_type {};
export template <class T, class... Args>
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

export template <std::endian EndianV>
using endian_in_place_tag = in_place_tag<std::endian, EndianV>;

} // namespace maboroutu
