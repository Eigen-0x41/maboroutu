#pragma once

#include "maboroutu/exbit.hpp"
#include "maboroutu/maboroutuDef.hpp"
#include "maboroutu/streamConcepts.hpp"
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <stdexcept>
namespace maboroutu {

class BinaryBuffer : public binary_t {
public:
  static_assert(!is_mix_endian_v, "is not mix endian.");
  static_assert(sizeof(value_type) == 1, "sizeof(value_type) is 1 byte.");

protected:
private:
private:
  difference_type Offset;

  template <std::unsigned_integral T, std::endian BinaryEndianV>
  T readCore(size_t Pos) const {
    if (Pos < Offset) [[unlikely]] {
      throw std::invalid_argument("Pos is lower than Offset.");
    }

    Pos -= Offset;
    if ((Pos + sizeof(T)) > size()) [[unlikely]] {
      throw std::range_error("is ((Pos  + sizeof(T)) > Size).");
    }

    if constexpr (sizeof(T) == 1) {
      return std::bit_cast<T>(operator[](Pos));
    }
    T RetValue = NULL;
    std::memcpy(&RetValue, data() + Pos, sizeof(T));
    if constexpr (BinaryEndianV != std::endian::native) {
      return std::byteswap(RetValue);
    }

    return RetValue;
  }

  template <std::unsigned_integral T, std::endian BinaryEndianV>
  constexpr T readCaster(size_t Pos) const {
    return readCore<T, BinaryEndianV>(Pos);
  }
  template <std::signed_integral T, std::endian BinaryEndianV>
  constexpr T readCaster(size_t Pos) const {
    static_assert(sizeof(T) <= 8, "Can conversion values.");
    if constexpr (sizeof(T) == 1) {
      return std::bit_cast<T>(readCore<uint8_t, BinaryEndianV>(Pos));
    }
    if constexpr (sizeof(T) == 2) {
      return std::bit_cast<T>(readCore<uint16_t, BinaryEndianV>(Pos));
    }
    if constexpr (sizeof(T) == 4) {
      return std::bit_cast<T>(readCore<uint32_t, BinaryEndianV>(Pos));
    }
    if constexpr (sizeof(T) == 8) {
      return std::bit_cast<T>(readCore<uint64_t, BinaryEndianV>(Pos));
    }
  }
  template <std::floating_point T, std::endian BinaryEndianV>
  constexpr T readCaster(size_t Pos) const {
    static_assert(sizeof(T) <= 8, "Can conversion values.");
    if constexpr (sizeof(T) == 1) {
      return std::bit_cast<T>(readCore<uint8_t, BinaryEndianV>(Pos));
    }
    if constexpr (sizeof(T) == 2) {
      return std::bit_cast<T>(readCore<uint16_t, BinaryEndianV>(Pos));
    }
    if constexpr (sizeof(T) == 4) {
      return std::bit_cast<T>(readCore<uint32_t, BinaryEndianV>(Pos));
    }
    if constexpr (sizeof(T) == 8) {
      return std::bit_cast<T>(readCore<uint64_t, BinaryEndianV>(Pos));
    }
  }
  template <typename T> constexpr T readCaster(size_t Pos) const {
    static_assert(sizeof(T) == 1, "Is 1Byte.");
    return std::bit_cast<T>(readCore<uint8_t, std::endian::native>(Pos));
  }

  template <std::unsigned_integral T, std::endian BinaryEndianV>
  void writeCore(size_t Pos, T Value) {
    if (Pos < Offset) [[unlikely]] {
      throw std::invalid_argument("Pos is lower than Offset.");
    }

    Pos -= Offset;

    if ((Pos + sizeof(T)) > size()) {
      resize(Pos + sizeof(T));
    }

    if constexpr (sizeof(T) == 1) {
      operator[](Pos) = std::bit_cast<value_type>(Value);
      return;
    }
    if constexpr (BinaryEndianV != std::endian::native) {
      Value = std::byteswap(Value);
    }
    std::memcpy(data() + Pos, &Value, sizeof(T));

    return;
  }

  template <std::unsigned_integral T, std::endian BinaryEndianV>
  constexpr void writeCaster(size_t Pos, T Value) {
    writeCore<T, BinaryEndianV>(Pos, Value);
    return;
  }
  template <std::signed_integral T, std::endian BinaryEndianV>
  constexpr void writeCaster(size_t Pos, T Value) {
    static_assert(sizeof(T) <= 8, "Can conversion values.");
    if constexpr (sizeof(T) == 1) {
      writeCore<uint8_t, BinaryEndianV>(Pos, std::bit_cast<uint8_t>(Value));
      return;
    }
    if constexpr (sizeof(T) == 2) {
      writeCore<uint16_t, BinaryEndianV>(Pos, std::bit_cast<uint16_t>(Value));
      return;
    }
    if constexpr (sizeof(T) == 4) {
      writeCore<uint32_t, BinaryEndianV>(Pos, std::bit_cast<uint32_t>(Value));
      return;
    }
    if constexpr (sizeof(T) == 8) {
      writeCore<uint64_t, BinaryEndianV>(Pos, std::bit_cast<uint64_t>(Value));
      return;
    }
  }
  template <std::floating_point T, std::endian BinaryEndianV>
  constexpr void writeCaster(size_t Pos, T Value) {
    static_assert(sizeof(T) <= 8, "Can conversion values.");
    if constexpr (sizeof(T) == 1) {
      writeCore<uint8_t, BinaryEndianV>(Pos, std::bit_cast<uint8_t>(Value));
      return;
    }
    if constexpr (sizeof(T) == 2) {
      writeCore<uint16_t, BinaryEndianV>(Pos, std::bit_cast<uint16_t>(Value));
      return;
    }
    if constexpr (sizeof(T) == 4) {
      writeCore<uint32_t, BinaryEndianV>(Pos, std::bit_cast<uint32_t>(Value));
      return;
    }
    if constexpr (sizeof(T) == 8) {
      writeCore<uint64_t, BinaryEndianV>(Pos, std::bit_cast<uint64_t>(Value));
      return;
    }
  }
  template <typename T> constexpr void writeCaster(size_t Pos, T Value) {
    static_assert(sizeof(T) == 1, "Is 1Byte.");
    writeCore<uint8_t, std::endian::little>(Pos, std::bit_cast<uint8_t>(Value));
    return;
  }

protected:
public:
  BinaryBuffer() : Offset(0), binary_t() {};
  BinaryBuffer(BinaryBuffer const &Rhs) = delete;
  BinaryBuffer(BinaryBuffer &&Rhs) = default;
  BinaryBuffer(size_type Size, size_type Offset = 0)
      : Offset(Offset), binary_t(Size) {};
  ~BinaryBuffer() {}
  BinaryBuffer &operator=(BinaryBuffer const &) = delete;
  BinaryBuffer &operator=(BinaryBuffer &&) = default;

  constexpr difference_type &offset() noexcept { return Offset; }
  constexpr difference_type const &offset() const noexcept { return Offset; }

  template <StreamIOConcept StIOT> void load(StIOT &Stream, size_type Size) {
    load(Stream, Size, Stream.fgetpos());
  }
  template <StreamIOConcept StIOT>
  void load(StIOT &Stream, size_type Size, size_type Offset) {
    this->Offset = Offset;
    resize(Size);
    resize(Stream.fread(data(), sizeof(value_type), Size));
  }

  template <StreamIOConcept StIOT>
  void store(StIOT &Stream, bool IsSyncOffset = true) {
    if (IsSyncOffset) {
      Stream.fsetpos(Offset);
    }
    Stream.fwrite(data(), sizeof(value_type), size());
  }

  template <typename T, std::integral PosT>
  constexpr T read(std::reference_wrapper<PosT> Pos) const {
    T RetValue = readCaster<T>(Pos.get());
    Pos.get() += sizeof(T);
    return RetValue;
  }
  template <typename T, std::integral PosT> T read(PosT Pos) const {
    return readCaster<T>(Pos);
  }
  template <typename T, std::endian BinaryEndianV, std::integral PosT>
  constexpr T read(std::reference_wrapper<PosT> Pos) const {
    T RetValue = readCaster<T, BinaryEndianV>(Pos.get());
    Pos.get() += sizeof(T);
    return RetValue;
  }
  template <typename T, std::endian BinaryEndianV, std::integral PosT>
  T read(PosT Pos) const {
    return readCaster<T, BinaryEndianV>(Pos);
  }

  template <typename T, std::integral PosT>
  constexpr void write(std::reference_wrapper<PosT> Pos, T Value) {
    writeCaster<T>(Pos.get(), Value);
    Pos.get() += sizeof(T);
    return;
  }
  template <typename T, std::integral PosT>
  constexpr void write(PosT Pos, T Value) {
    writeCaster<T>(Pos, Value);
    return;
  }
  template <typename T, std::endian BinaryEndianV, std::integral PosT>
  constexpr void write(std::reference_wrapper<PosT> const Pos, T Value) {
    writeCaster<T, BinaryEndianV>(Pos.get(), Value);
    Pos.get() += sizeof(T);
    return;
  }
  template <typename T, std::endian BinaryEndianV, std::integral PosT>
  constexpr void write(PosT Pos, T Value) {
    writeCaster<T, BinaryEndianV>(Pos, Value);
    return;
  }

  constexpr ret<std::reference_wrapper<value_type>>
  tryAt(size_type Pos) noexcept {
    if (Pos >= size()) [[unlikely]] {
      return makeRetErr<ret<>::error_type>(
          ret<>::error_type::categoly_type::Logic,
          ret<>::error_type::descript_type::OutOfRange, "Pos >= BufferSize");
    }
    return std::ref(operator[](Pos));
  }
  constexpr ret<std::reference_wrapper<value_type const>>
  tryRawAccess(size_type Pos) const noexcept {
    if (Pos >= size()) [[unlikely]] {
      return makeRetErr<ret<>::error_type>(
          ret<>::error_type::categoly_type::Logic,
          ret<>::error_type::descript_type::OutOfRange, "Pos >= BufferSize");
    }
    return std::ref(operator[](Pos));
  }
};

} // namespace maboroutu
