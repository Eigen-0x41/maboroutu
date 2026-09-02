module;
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <span>
#include <type_traits>
#include <vector>
export module maboroutu.binary_convert;
export import maboroutu.core;
export import maboroutu.error;
export import maboroutu.data_source;

namespace maboroutu {

export using endian = std::endian;

export template <class T>
concept numberable =
    std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T>;

namespace binary_convert_detail {

// NOTE:
// 各uintXX_t系が定義されていない場合はuint_least8_tをスタブとして定義する。
#if defined(UINT8_MAX)
using u8_t = std::uint8_t;
inline constexpr bool has_u8 = true;
#else
using u8_t = std::uint_least8_t;
inline constexpr bool has_u8 = false;
#endif

#if defined(UINT16_MAX)
using u16_t = std::uint16_t;
inline constexpr bool has_u16 = true;
#else
using u16_t = std::uint_least8_t;
inline constexpr bool has_u16 = false;
#endif

#if defined(UINT32_MAX)
using u32_t = std::uint32_t;
inline constexpr bool has_u32 = true;
#else
using u32_t = std::uint_least8_t;
inline constexpr bool has_u32 = false;
#endif

#if defined(UINT64_MAX)
using u64_t = std::uint64_t;
inline constexpr bool has_u64 = true;
#else
using u64_t = std::uint_least8_t;
inline constexpr bool has_u64 = false;
#endif

// NOTE: std::uint128_t はC++23/26の規格に存在せず、UINT128_MAXも
// 標準では定義されない。将来の規格拡張に備えた分岐であり、現行の主要
// コンパイラでは常に has_u128 == false となる
#if defined(UINT128_MAX)
using u128_t = std::uint128_t;
inline constexpr bool has_u128 = true;
#else
using u128_t = std::uint8_t;
inline constexpr bool has_u128 = false;
#endif

template <class T>
concept byteswappable_size =
    numberable<T> && ((has_u8 && (sizeof(T) == sizeof(u8_t))) ||
                      (has_u16 && (sizeof(T) == sizeof(u16_t))) ||
                      (has_u32 && (sizeof(T) == sizeof(u32_t))) ||
                      (has_u64 && (sizeof(T) == sizeof(u64_t))) ||
                      (has_u128 && (sizeof(T) == sizeof(u128_t))));

// std::byteswapをstd::bit_castを挟むことでbit値が同じ型なら変換可能にする。
template <byteswappable_size T>
constexpr auto wrap_byteswap(T value) noexcept -> T {
   if constexpr (has_u8 && (sizeof(T) == sizeof(u8_t))) {
      return value;
   } else if constexpr (has_u16 && (sizeof(T) == sizeof(u16_t))) {
      return std::bit_cast<T>(std::byteswap(std::bit_cast<u16_t>(value)));
   } else if constexpr (has_u32 && (sizeof(T) == sizeof(u32_t))) {
      return std::bit_cast<T>(std::byteswap(std::bit_cast<u32_t>(value)));
   } else if constexpr (has_u64 && (sizeof(T) == sizeof(u64_t))) {
      return std::bit_cast<T>(std::byteswap(std::bit_cast<u64_t>(value)));
   } else {
      static_assert(has_u128 && (sizeof(T) == sizeof(u128_t)),
                    "T is not compatible wrap_byteswap.");
      return std::bit_cast<T>(std::byteswap(std::bit_cast<u128_t>(value)));
   }
} // namespace binary_convert_detail

export template <endian Endian, numberable T>
[[nodiscard]] constexpr auto
from_bytes(std::array<std::byte, sizeof(T)> const &bytes) noexcept -> T {
   T value = std::bit_cast<T>(bytes);
   if constexpr (sizeof(T) != 1 && Endian != endian::native) {
      static_assert(endian::native == endian::big ||
                        endian::native == endian::little,
                    "not supported endian.");
      value = binary_convert_detail::wrap_byteswap(value);
   }
   return value;
}
export template <endian Endian, numberable T>
[[nodiscard]] constexpr auto to_bytes(T value) noexcept
    -> std::array<std::byte, sizeof(T)> {
   if constexpr (sizeof(T) != 1 && Endian != endian::native) {
      static_assert(endian::native == endian::big ||
                        endian::native == endian::little,
                    "not supported endian.");
      value = binary_convert_detail::wrap_byteswap(value);
   }
   return std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
}

// data_source からの単一値読み込み。
export template <endian Endian, numberable T, data_source Src>
[[nodiscard]] auto read_value(Src &src, std::size_t offset)
    -> data_source_result<T> {
   auto bytes = src.read(region{
       .offset = offset,
       .size = sizeof(T),
   });
   if (!bytes) {
      return std::unexpected(bytes.error());
   }
   assert(bytes->size == sizeof(T) &&
          "data_source::read() violated its no-short-read contract.");
   std::array<std::byte, sizeof(T)> buf{};
   std::memcpy(buf.data(), bytes->value.get(), sizeof(T));
   return from_bytes<Endian, T>(buf);
}

// data_source への単一値書き込み。
export template <endian Endian, numberable T, writable_data_source Src>
[[nodiscard]] auto write_value(Src &dst, std::size_t offset, T value)
    -> data_source_result<void> {
   auto const bytes = to_bytes<Endian, T>(value);
   return dst.write(
       region{
           .offset = offset,
           .size = sizeof(T),
       },
       std::span<std::byte const>(bytes.data(), bytes.size()));
}

// data_source からの固定長配列読み込み。
export template <endian Endian, numberable T, std::size_t Size, data_source Src>
[[nodiscard]] auto read_array(Src &src, std::size_t offset)
    -> data_source_result<std::array<T, Size>> {
   auto bytes = src.read(region{
       .offset = offset,
       .size = sizeof(T) * Size,
   });
   if (!bytes) {
      return std::unexpected(bytes.error());
   }
   assert(bytes->size == sizeof(T) * Size &&
          "data_source::read() violated its no-short-read contract.");
   std::array<T, Size> ret_value{};
   for (std::size_t i = 0; i < Size; ++i) {
      std::array<std::byte, sizeof(T)> buf{};
      std::memcpy(buf.data(), bytes->value.get() + (i * sizeof(T)), sizeof(T));
      ret_value[i] = from_bytes<Endian, T>(buf);
   }
   return ret_value;
}

// data_source への固定長配列書き込み。
export template <endian Endian, numberable T, std::size_t Size,
                 writable_data_source Src>
[[nodiscard]] auto write_array(Src &dst, std::size_t offset,
                               std::array<T, Size> const &values)
    -> data_source_result<void> {
   std::array<std::byte, sizeof(T) * Size> buf{};
   for (std::size_t i = 0; i < Size; ++i) {
      auto const encoded = to_bytes<Endian, T>(values[i]);
      std::memcpy(buf.data() + (i * sizeof(T)), encoded.data(), sizeof(T));
   }
   return dst.write(
       region{
           .offset = offset,
           .size = sizeof(T) * Size,
       },
       std::span<std::byte const>(buf.data(), buf.size()));
}

// data_source からの可変長読み込み。
export template <endian Endian, numberable T, data_source Src>
[[nodiscard]] auto read_vector(Src &src, std::size_t offset, std::size_t count)
    -> data_source_result<std::vector<T>> {
   auto bytes = src.read(region{
       .offset = offset,
       .size = sizeof(T) * count,
   });
   if (!bytes) {
      return std::unexpected(bytes.error());
   }
   assert(bytes->size == sizeof(T) * count &&
          "data_source::read() violated its no-short-read contract.");
   std::vector<T> ret_value;
   ret_value.reserve(count);
   for (std::size_t i = 0; i < count; ++i) {
      std::array<std::byte, sizeof(T)> buf{};
      std::memcpy(buf.data(), bytes->value.get() + (i * sizeof(T)), sizeof(T));
      ret_value.push_back(from_bytes<Endian, T>(buf));
   }
   return ret_value;
}

// data_source への可変長書き込み。
export template <endian Endian, numberable T, writable_data_source Src>
[[nodiscard]] auto write_vector(Src &dst, std::size_t offset,
                                std::vector<T> const &values)
    -> data_source_result<void> {
   std::vector<std::byte> buf(sizeof(T) * values.size());
   for (std::size_t i = 0; i < values.size(); ++i) {
      auto const encoded = to_bytes<Endian, T>(values[i]);
      std::memcpy(buf.data() + (i * sizeof(T)), encoded.data(), sizeof(T));
   }
   return dst.write(
       region{
           .offset = offset,
           .size = buf.size(),
       },
       std::span<std::byte const>(buf.data(), buf.size()));
}

} // namespace maboroutu
