module;
#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <istream>
#include <vector>

export module maboroutu.binarystream:basic_iobinarystream;
import :common;

namespace maboroutu {
export template <class CharT, class Traits = std::char_traits<CharT>>
class basic_iobinarystream : public std::basic_iostream<CharT, Traits> {
public: /*STRUCT_FIELD*/
protected:
private:
  using self_type = basic_iobinarystream;
  using base_type = typename std::basic_iostream<CharT, Traits>;

  template <class T, size_t Size>
  using convert_array =
      make_convert_array<typename base_type::char_type, T, Size>;
  template <class T>
  using convert_vector = make_convert_vector<typename base_type::char_type, T>;

  /*      IMPLIMENT_FIELD*/
  template <class T, size_t Size>
  auto _read_convert_array(this base_type &self) ->
      typename convert_array<T, Size>::value_type {
    auto ret_value = convert_array<T, Size>::make_convert_table();
    self.read(ret_value.data(), ret_value.size());
    return ret_value;
  }
  template <class T>
  auto _read_convert_vector(this base_type &self, size_t size) ->
      typename convert_vector<T>::value_type {
    auto ret_value = convert_vector<T>::make_convert_table(size);
    self.read(ret_value.data(), ret_value.size());
    return ret_value;
  }
  template <class T, size_t Size>
  auto _write_convert_array(
      this base_type &self,
      typename convert_array<T, Size>::value_type const &convert_array)
      -> void {
    self.write(convert_array.data(), convert_array.size());
  }
  template <class T>
  auto _write_convert_vector(
      this base_type &self,
      typename convert_vector<T>::value_type const &convert_array) -> void {
    self.write(convert_array.data(), convert_array.size());
  }

protected:
public:
  using base_type::basic_iostream;
  basic_iobinarystream(const basic_iobinarystream &) = delete;
  basic_iobinarystream(basic_iobinarystream &&) = delete;

  virtual ~basic_iobinarystream() = default;

  auto operator=(basic_iobinarystream const &)
      -> basic_iobinarystream & = default;
  auto operator=(basic_iobinarystream &&) -> basic_iobinarystream & = default;

  using base_type::read;
  template <std::endian Endian, std::integral ValueT>
  auto read(this self_type &self) -> ValueT {
    ValueT ret_value = convert_array<ValueT, 1>::from(
        self._read_convert_array<ValueT, 1>())[0];

    if constexpr (sizeof(ValueT) != 1) {
      if constexpr (Endian != std::endian::native) {
        static_assert(std::endian::native == std::endian::big ||
                          std::endian::native == std::endian::little,
                      "not suppeort endian.");
        return std::byteswap(ret_value);
      }
    }
    return ret_value;
  }
  template <std::endian Endian, std::floating_point FloatingPointT>
  auto read(this self_type &self) -> FloatingPointT {
    if constexpr (sizeof(FloatingPointT) == sizeof(uint32_t)) {
      return std::bit_cast<FloatingPointT>(self.read<Endian, uint32_t>());
    }
    if constexpr (sizeof(FloatingPointT) == sizeof(uint64_t)) {
      return std::bit_cast<FloatingPointT>(self.read<Endian, uint64_t>());
    } else {
      static_assert(true, "value is not support.");
    }
  }
  template <std::endian Endian, std::integral ValueT, size_t Size>
  auto read_array(this self_type &self) -> std::array<ValueT, Size> {
    std::array<ValueT, Size> ret_value = convert_array<ValueT, Size>::from(
        self._read_convert_array<ValueT, Size>());

    if constexpr (sizeof(ValueT) != 1) {
      if constexpr (Endian != std::endian::native) {
        static_assert(std::endian::native == std::endian::big ||
                          std::endian::native == std::endian::little,
                      "not suppeort endian.");
        member_byte_swap(ret_value);
      }
    }
    return ret_value;
  }
  template <std::endian Endian, std::integral ValueT>
  auto read_vector(this self_type &self, size_t size) -> std::vector<ValueT> {
    std::vector<ValueT> ret_value =
        convert_vector<ValueT>::from(self._read_convert_vector<ValueT>(size));

    if constexpr (sizeof(ValueT) != 1) {
      if constexpr (Endian != std::endian::native) {
        static_assert(std::endian::native == std::endian::big ||
                          std::endian::native == std::endian::little,
                      "not suppeort endian.");
        member_byte_swap(ret_value);
      }
    }
    return ret_value;
  }

  using base_type::write;
  template <std::endian Endian, std::integral ValueT>
  void write(this self_type &self, ValueT value) {
    if constexpr (Endian != std::endian::native) {
      static_assert(std::endian::native == std::endian::big ||
                        std::endian::native == std::endian::little,
                    "Can convert endian.");

      value = std::byteswap(value);
    }

    self._write_convert_array<ValueT, 1>(convert_array<ValueT, 1>::to(value));
  }
  template <std::endian Endian, std::floating_point ValueT>
  void write(this self_type &self, ValueT floating_point_v) {
    if constexpr (sizeof(ValueT) == sizeof(uint32_t)) {
      self.write<Endian, uint32_t>(std::bit_cast<uint32_t>(floating_point_v));
    }
    if constexpr (sizeof(ValueT) == sizeof(uint64_t)) {
      self.write<Endian, uint64_t>(std::bit_cast<uint64_t>(floating_point_v));
    } else {
      static_assert(true, "value is not support.");
    }
  }
  template <std::endian Endian, std::integral ValueT, size_t Size>
  void write_array(this self_type &self, std::array<ValueT, Size> value) {
    if constexpr (sizeof(ValueT) != 1) {
      if constexpr (Endian != std::endian::native) {
        static_assert(std::endian::native == std::endian::big ||
                          std::endian::native == std::endian::little,
                      "not suppeort endian.");
        member_byte_swap(value);
      }
    }

    self._write_convert_array<ValueT, Size>(
        convert_array<ValueT, Size>::to(value));
  }
  template <std::endian Endian, std::integral ValueT>
  void write_vector(this self_type &self, std::vector<ValueT> value) {
    if constexpr (sizeof(ValueT) != 1) {
      if constexpr (Endian != std::endian::native) {
        static_assert(std::endian::native == std::endian::big ||
                          std::endian::native == std::endian::little,
                      "not suppeort endian.");
        member_byte_swap(value);
      }
    }

    self._write_convert_vector<ValueT>(convert_vector<ValueT>::to(value));
  }
};

} // namespace maboroutu
