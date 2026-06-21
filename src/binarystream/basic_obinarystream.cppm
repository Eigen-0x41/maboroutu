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

export module maboroutu.binarystream:basic_obinarystream;
import :common;

namespace maboroutu {
export template <class CharT, class Traits = std::char_traits<CharT>>
class basic_obinarystream : public std::basic_ostream<CharT, Traits> {
public: /*STRUCT_FIELD*/
protected:
private:
  using self_type = basic_obinarystream;
  using base_type = typename std::basic_ostream<CharT, Traits>;

  template <class T, size_t Size>
  using convert_array =
      make_convert_array<typename base_type::char_type, T, Size>;
  template <class T>
  using convert_vector = make_convert_vector<typename base_type::char_type, T>;

  /*--:  *IMPLIMENT_FIELD*/
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
  using base_type::basic_ostream;
  basic_obinarystream(const basic_obinarystream &) = delete;
  basic_obinarystream(basic_obinarystream &&) = delete;

  virtual ~basic_obinarystream() = default;

  auto operator=(basic_obinarystream const &)
      -> basic_obinarystream & = default;
  auto operator=(basic_obinarystream &&) -> basic_obinarystream & = default;

  using base_type::write;
  template <std::endian Endian, numberable ValueT>
  void write(this self_type &self, ValueT value) {
    if constexpr (Endian != std::endian::native) {
      static_assert(std::endian::native == std::endian::big ||
                        std::endian::native == std::endian::little,
                    "Can convert endian.");

      value = std::byteswap(value);
    }

    self._write_convert_array<ValueT, 1>(convert_array<ValueT, 1>::to(value));
  }
  void write_byte(this self_type &self, std::byte value) {
    static_assert(sizeof(std::byte) == 1, "std::byte is not 1byte.");
    self._write_convert_array<std::byte, 1>(
        convert_array<std::byte, 1>::to(value));
  }
  template <std::endian Endian, numberable ValueT, size_t Size>
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
  template <size_t Size>
  void write_byte_array(this self_type &self,
                        std::array<std::byte, Size> value) {
    static_assert(sizeof(std::byte) == 1, "std::byte is not 1byte.");
    self._write_convert_array<std::byte, Size>(
        convert_array<std::byte, Size>::to(value));
  }
  template <std::endian Endian, numberable ValueT>
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
  void write_byte_array(this self_type &self, std::vector<std::byte> value) {
    static_assert(sizeof(std::byte) == 1, "std::byte is not 1byte.");
    self._write_convert_vector<std::byte>(convert_vector<std::byte>::to(value));
  }
};

} // namespace maboroutu
