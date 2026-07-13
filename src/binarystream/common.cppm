module;
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ratio>
#include <type_traits>
#include <vector>

export module maboroutu.binarystream:common;

export namespace maboroutu {
template <class CharT, class T, size_t Size = 1> struct make_convert_array {
   using ratio = std::ratio<sizeof(CharT), sizeof(T)>;
   static_assert(((Size * ratio::den) % ratio::num) == 0,
                 "can not convert table.");
   using value_type =
       typename std::array<CharT, (Size * ratio::den) / ratio::num>;
   using target_type = typename std::array<T, Size>;

   constexpr static auto make_convert_table() -> value_type { return {}; }

   constexpr static auto from(value_type const &value) noexcept -> target_type {
      return std::bit_cast<target_type>(value);
   }
   constexpr static auto to(target_type const &value) noexcept -> value_type {
      return std::bit_cast<value_type>(value);
   }
   constexpr static auto to(T const value) noexcept -> value_type {
      return std::bit_cast<value_type>(value);
   }
};
template <class CharT, class T> struct make_convert_vector {
   using ratio = std::ratio<sizeof(CharT), sizeof(T)>;
   using value_type = typename std::vector<CharT>;
   using target_type = typename std::vector<T>;

   constexpr static auto make_convert_table(size_t size) -> value_type {
      assert(((size * ratio::den) % ratio::num) == 0);
      return value_type((size * ratio::den) / ratio::num, {});
   }

   constexpr static auto from(value_type const &value) -> target_type {
      assert(((value.size() * ratio::num) % ratio::den) == 0);
      target_type ret_value((value.size() * ratio::num) / ratio::den, {});
      std::memcpy(ret_value.data(), value.data(), value.size() * ratio::num);
      return ret_value;
   }
   constexpr static auto to(target_type const &value) -> value_type {
      value_type ret_value = make_convert_table(value.size());
      std::memcpy(ret_value.data(), value.data(), value.size() * ratio::den);
      return ret_value;
   }
};

template <class T> constexpr auto wrap_byteswap(T value) -> T {
#if defined(UINT8_MAX)
   if constexpr (sizeof(T) == sizeof(uint8_t)) {
      return std::bit_cast<T>(std::byteswap(std::bit_cast<uint8_t>(value)));
   }
#endif
#if defined(UINT16_MAX)
   if constexpr (sizeof(T) == sizeof(uint16_t)) {
      return std::bit_cast<T>(std::byteswap(std::bit_cast<uint16_t>(value)));
   }
#endif
#if defined(UINT32_MAX)
   if constexpr (sizeof(T) == sizeof(uint32_t)) {
      return std::bit_cast<T>(std::byteswap(std::bit_cast<uint32_t>(value)));
   }
#endif
#if defined(UINT64_MAX)
   if constexpr (sizeof(T) == sizeof(uint64_t)) {
      return std::bit_cast<T>(std::byteswap(std::bit_cast<uint64_t>(value)));
   }
#endif
#if defined(UINT128_MAX)
   if constexpr (sizeof(T) == sizeof(uint128_t)) {
      return std::bit_cast<T>(std::byteswap(std::bit_cast<uint128_t>(value)));
   }
#endif
   assert(false);
}

template <class T>
concept numberable =
    std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T>;

template <class T> constexpr void member_byte_swap(T &value) {
#pragma unroll 4
   for (numberable auto &member : value) {
      member = wrap_byteswap(member);
   }
}
} // namespace maboroutu
