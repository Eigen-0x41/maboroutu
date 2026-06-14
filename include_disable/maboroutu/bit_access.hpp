#pragma once

#include <bit>
#include <climits>
#include <concepts>
#include <cstdlib>
#include <stdexcept>
#include <type_traits>
namespace maboroutu {

template <std::integral IntegralT, size_t BitPosV, size_t BitSizeV>
struct bitfield_traits {
  using value_type = IntegralT;
  using unsigned_value_type = typename std::make_unsigned_t<IntegralT>;

  template <size_t LocBitPosV, size_t LocBitSizeV>
  using make_this = bitfield_traits<value_type, LocBitPosV, LocBitSizeV>;

  static constexpr const size_t bitsizeof_value = sizeof(value_type) * CHAR_BIT;
  static constexpr const size_t pos = BitPosV;
  static constexpr const size_t size = BitSizeV;
  static constexpr const size_t shift_number = (bitsizeof_value - size) - pos;

  static_assert(pos + size < bitsizeof_value, "do not overflow accesser.");

  [[nodiscard]] static constexpr auto value_filter() noexcept
      -> unsigned_value_type {
    constexpr unsigned_value_type filter_base = (0x01 << (size - 1));
    if constexpr (std::is_unsigned_v<value_type>) {
      return filter_base | (filter_base - 1);
    } else {
      return filter_base - 1;
    }
  }
  [[nodiscard]] static constexpr auto sign_filter() noexcept
      -> unsigned_value_type {
    if constexpr (std::is_unsigned_v<value_type>) {
      return 0;
    } else {
      return value_filter() + 1;
    }
  }
  [[nodiscard]] static constexpr auto filter() noexcept -> unsigned_value_type {
    return sign_filter() | value_filter();
  }

  [[nodiscard]] static constexpr auto
  can_serialize(value_type const value) noexcept -> bool {
    if constexpr (std::is_signed_v<value_type>) {
      return std::abs(value) <= value_filter();
    }
    return value <= value_filter();
  }

  /**
   * @brief
   * You call can_serialize when before calling this function if you need
   * checking.
   *
   * @param value [TODO:parameter]
   * @return [TODO:return]
   */
  static constexpr void serialize(value_type &base,
                                  value_type const value) noexcept {
    auto const unsigned_value = std::bit_cast<unsigned_value_type>(value);
    auto unsigned_base = std::bit_cast<unsigned_value_type>(base);

    unsigned_base = std::rotr(unsigned_base, shift_number);
    unsigned_base = unsigned_base & ~value_filter();
    unsigned_base = unsigned_base | (unsigned_value & value_filter());

    base = std::bit_cast<value_type>(std::rotl(unsigned_base, shift_number));
  }
  [[nodiscard]] static constexpr auto
  deserialize(value_type const base) noexcept -> value_type {
    auto const unsigned_base =
        std::rotr(std::bit_cast<unsigned_value_type>(base), shift_number);

    unsigned_value_type ret_unsignd_value = unsigned_base & value_filter();
    if ((unsigned_base & sign_filter()) != 0) {
      ret_unsignd_value = ~value_filter() | ret_unsignd_value;
    }

    return std::bit_cast<value_type>(ret_unsignd_value);
  }
};

template <std::integral IntegralT, class BitfieldTraits>
class basic_bitfield_access {
public: /*STRUCT_FIELD*/
  using value_type = IntegralT;
  using traits_type = BitfieldTraits;

protected:
private:
  static constexpr traits_type trait;

  std::reference_wrapper<value_type> _value;

  /*      IMPLIMENT_FIELD*/
protected:
public:
  basic_bitfield_access() = delete;
  basic_bitfield_access(std::reference_wrapper<value_type> value)
      : _value(value) {}
  basic_bitfield_access(basic_bitfield_access const &rhs) = default;
  basic_bitfield_access(basic_bitfield_access &&rhs) = default;
  ~basic_bitfield_access() = default;

  auto operator=(basic_bitfield_access const &)
      -> basic_bitfield_access & = default;
  auto operator=(basic_bitfield_access &&) -> basic_bitfield_access & = default;

  constexpr auto get() const noexcept -> value_type {
    return trait.deserialize(_value.get());
  }

  constexpr void set(value_type value) {
    if (!trait.can_serialize(value)) [[unlikely]] {
      throw std::overflow_error("can not serialize because overflow.");
    }
    trait.serialize(_value.get(), value);
  }

  constexpr auto operator=(value_type value) noexcept
      -> basic_bitfield_access & {
    trait.serialize(_value.get(), value);
    return *this;
  }

  constexpr operator value_type() const noexcept {
    return trait.deserialize(_value.get());
  }
};

// template <size_t BitPosV, size_t BitSizeV, class T,
//           class BitfieldTraits = bitfield_traits<T, BitPosV, BitSizeV>>
// auto make_bitfield_access(std::reference_wrapper<T> refwrap) {
//   return bitfield_access<T, BitPosV, BitSizeV, BitfieldTraits>(refwrap);
// }
// template <class T, size_t BitSizeV>
// auto make_bitfield_access(std::reference_wrapper<T> refwrap) {
//   using traits_type = T::traits_type;
//   size_t const bit_position = traits_type::pos() + traits_type::size();
//   return bitfield_access<typename T::value_type, bit_position, BitSizeV,
//                          traits_type>(refwrap);
// }

template <class T, size_t BitPosV, size_t BitSizeV>
using bitfield_access =
    basic_bitfield_access<T, bitfield_traits<T, BitPosV, BitSizeV>>;

template <class T, size_t BitSizeV>
using depending_bitfield_access = basic_bitfield_access<
    typename T::value_type,
    typename T::traits_type::template make_this<
        T::traits_type::pos + T::traits_type::size, BitSizeV>>;

} // namespace maboroutu
