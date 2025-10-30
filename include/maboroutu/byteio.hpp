#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstdint>
#include <fstream>

namespace maboroutu {
template <class StreamDerived> class basic_binary_stream {
public: /*STRUCT_FIELD*/
  using value_type = StreamDerived;

  using char_type = typename value_type::char_type;
  using traits_type = typename value_type::traits_type;

protected:
private:
  template <class T>
  using convert_array_type = typename std::array<char_type, sizeof(T)>;

  value_type _fs;

  /*      IMPLIMENT_FIELD*/
protected:
public:
  basic_binary_stream() = delete;
  basic_binary_stream(basic_binary_stream const &rhs) = delete;
  basic_binary_stream(basic_binary_stream &&rhs) noexcept
      : _fs(std::move(rhs._fs)) {}
  basic_binary_stream(value_type &&fstream) noexcept
      : _fs(std::move(fstream)) {}
  ~basic_binary_stream() = default;

  auto operator=(basic_binary_stream const &)
      -> basic_binary_stream & = default;
  auto operator=(basic_binary_stream &&) -> basic_binary_stream & = default;

  constexpr auto value() & noexcept -> value_type & { return _fs; }
  constexpr auto value() const & noexcept -> const value_type & { return _fs; }
  constexpr auto value() && noexcept -> value_type && { return _fs; }
  constexpr auto value() const && noexcept -> const value_type && {
    return _fs;
  }

  template <std::endian EndianV, std::integral IntegralT>
  auto read() -> IntegralT {
    convert_array_type<IntegralT> convert_array = {};
    _fs.read(convert_array.data(), convert_array.size());

    auto buffer = std::bit_cast<IntegralT>(convert_array);

    if constexpr (EndianV != std::endian::native) {
      static_assert(std::endian::native == std::endian::big ||
                        std::endian::native == std::endian::little,
                    "Can convert endian.");

      return std::byteswap(buffer);
    }
    return buffer;
  }
  template <std::endian EndianV, std::floating_point FloatingPointT>
  auto read() -> FloatingPointT {
    if constexpr (sizeof(FloatingPointT) == sizeof(uint32_t)) {
      return std::bit_cast<FloatingPointT>(read<EndianV, uint32_t>());
    }
    if constexpr (sizeof(FloatingPointT) == sizeof(uint64_t)) {
      return std::bit_cast<FloatingPointT>(read<EndianV, uint64_t>());
    } else {
      static_assert(true, "value is not support.");
    }
  }

  template <std::endian EndianV, std::integral IntegralT>
  void write(IntegralT integral_v) {
    if constexpr (EndianV != std::endian::native) {
      static_assert(std::endian::native == std::endian::big ||
                        std::endian::native == std::endian::little,
                    "Can convert endian.");

      integral_v = std::byteswap(integral_v);
    }

    auto convert_array =
        std::bit_cast<convert_array_type<IntegralT>>(integral_v);

    _fs.write(convert_array.data(), convert_array.size());
  }
  template <std::endian EndianV, std::floating_point FloatingPointT>
  void write(FloatingPointT floating_point_v) {
    if constexpr (sizeof(FloatingPointT) == sizeof(uint32_t)) {
      write<EndianV, uint32_t>(std::bit_cast<uint32_t>(floating_point_v));
    }
    if constexpr (sizeof(FloatingPointT) == sizeof(uint64_t)) {
      write<EndianV, uint64_t>(std::bit_cast<uint32_t>(floating_point_v));
    } else {
      static_assert(true, "value is not support.");
    }
  }
};

template <std::derived_from<std::fstream> StreamDerived>
using binary_stream = basic_binary_stream<StreamDerived>;
template <std::derived_from<std::ifstream> IStreamDerived>
using binary_istream = basic_binary_stream<IStreamDerived>;
template <std::derived_from<std::ifstream> OStreamDerived>
using binary_ostream = basic_binary_stream<OStreamDerived>;
} // namespace maboroutu
