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

export module maboroutu.binarystream:basic_ibinarystream;
import :common;

namespace maboroutu {
export template <class CharT, class Traits = std::char_traits<CharT>>
class basic_ibinarystream : public std::basic_istream<CharT, Traits> {
public: /*STRUCT_FIELD*/
protected:
private:
  using self_type = basic_ibinarystream;
  using base_type = typename std::basic_istream<CharT, Traits>;

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

protected:
public:
  using base_type::basic_istream;
  basic_ibinarystream(const basic_ibinarystream &) = delete;
  basic_ibinarystream(basic_ibinarystream &&) = delete;

  virtual ~basic_ibinarystream() = default;

  auto operator=(basic_ibinarystream const &)
      -> basic_ibinarystream & = default;
  auto operator=(basic_ibinarystream &&) -> basic_ibinarystream & = default;

  using base_type::read;
  template <std::endian Endian, numberable ValueT>
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
  auto read_byte(this self_type &self) -> std::byte {
    static_assert(sizeof(std::byte) == 1, "std::byte is not 1byte.");
    return convert_array<std::byte, 1>::from(
        self._read_convert_array<std::byte, 1>())[0];
  }
  template <std::endian Endian, numberable ValueT, size_t Size>
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
  template <size_t Size>
  auto read_byte_array(this self_type &self) -> std::array<std::byte, Size> {
    static_assert(sizeof(std::byte) == 1, "std::byte is not 1byte.");
    return convert_array<std::byte, Size>::from(
        self._read_convert_array<std::byte, Size>());
  }
  template <std::endian Endian, numberable ValueT>
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
  auto read_byte_vector(this self_type &self, size_t size)
      -> std::vector<std::byte> {
    static_assert(sizeof(std::byte) == 1, "std::byte is not 1byte.");
    return convert_vector<std::byte>::from(
        self._read_convert_vector<std::byte>(size));
  }
};

} // namespace maboroutu
