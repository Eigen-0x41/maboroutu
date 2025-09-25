#pragma once

#include "maboroutu/exbit.hpp"
#include "maboroutu/maboroutuDef.hpp"
#include "maboroutu/streamConcepts.hpp"
#include <bit>
#include <cassert>
#include <climits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#include <stdexcept>
namespace maboroutu {

class BinaryBuffer {
private:
protected:
public:
  using this_type = BinaryBuffer;
  using value_type = byte_t;
  using size_type = int64_t;

  static_assert(!is_mix_endian_v, "is not mix endian.");

private:
  size_type Offset;
  size_type Size;
  std::unique_ptr<value_type[]> Buffer;
  size_type CapacitySize;

  constexpr void rawWrite(value_type Value, size_type RawPos) {
    if (RawPos >= Size) [[unlikely]] {
      throw std::invalid_argument("");
    }
    Buffer[RawPos] = Value;
  }

  [[nodiscard]] constexpr value_type rawRead(size_type RawPos) const {
    if (RawPos >= Size) [[unlikely]] {
      throw std::invalid_argument("");
    }
    return Buffer[RawPos];
  }

protected:
public:
  BinaryBuffer() : Offset(0), Size(0), Buffer(nullptr), CapacitySize(0) {};
  BinaryBuffer(this_type const &Rhs) = delete;
  BinaryBuffer(this_type &&Rhs) = default;
  BinaryBuffer(size_type Size, size_type Offset = 0)
      : Offset(Offset), Size(Size),
        Buffer(std::make_unique<value_type[]>(Size)), CapacitySize(Size) {};
  ~BinaryBuffer() {}
  this_type &operator=(this_type const &) = delete;
  this_type &operator=(this_type &&) = default;

  constexpr size_type size() const noexcept { return Size; }
  constexpr size_type capacitySize() const noexcept { return CapacitySize; }

  template <StreamIOConcept StIOT>
  void load(StIOT &Stream, size_type CapacitySize) {
    load(Stream, CapacitySize, Stream.fgetpos());
  }
  template <StreamIOConcept StIOT>
  void load(StIOT &Stream, size_type CapacitySize, size_type Offset) {
    this->Offset = Offset;
    this->CapacitySize = CapacitySize;
    Buffer = std::make_unique<value_type[]>(CapacitySize);
    Size = Stream.fread(Buffer.get(), sizeof(value_type),
                        CapacitySize * sizeof(value_type));
  }
  template <StreamIOConcept StIOT>
  void store(StIOT &Stream, bool IsSyncPos = false) {
    if (IsSyncPos) {
      Stream.fsetpos(Offset);
    }
    Size = Stream.fwrite(Buffer.get(), sizeof(value_type),
                         CapacitySize * sizeof(value_type));
  }

  template <std::unsigned_integral T, std::endian BinaryEndianV>
  T read(size_t Pos) const {
    if (Pos < Offset) [[unlikely]] {
      throw std::invalid_argument("Pos is lower than Offset.");
    }

    auto RawPos = Pos - Offset;
    if ((RawPos + sizeof(T)) >= Size) [[unlikely]] {
      throw std::range_error("is ((Pos  + sizeof(T)) >= Size).");
    }

    if constexpr (sizeof(T) > 1) {
      T RetValue = NULL;
      std::memcpy(&RetValue, Buffer.get() + Pos, sizeof(T));

      if constexpr (BinaryEndianV != std::endian::native) {
        return std::byteswap(RetValue);
      }
      return RetValue;
    }

    return std::bit_cast<T>(Buffer[Pos]);
  }
  template <std::signed_integral T, std::endian BinaryEndianV>
  constexpr T read(size_t Pos) const {
    static_assert(sizeof(T) <= 8, "Can conversion values.");
    if constexpr (sizeof(T) == 1) {
      return std::bit_cast<T>(read<uint8_t, BinaryEndianV>(Pos));
    }
    if constexpr (sizeof(T) == 2) {
      return std::bit_cast<T>(read<uint16_t, BinaryEndianV>(Pos));
    }
    if constexpr (sizeof(T) == 4) {
      return std::bit_cast<T>(read<uint32_t, BinaryEndianV>(Pos));
    }
    if constexpr (sizeof(T) == 8) {
      return std::bit_cast<T>(read<uint64_t, BinaryEndianV>(Pos));
    }
  }
  template <std::floating_point T, std::endian BinaryEndianV>
  constexpr T read(size_t Pos) const {
    static_assert(sizeof(T) <= 8, "Can conversion values.");
    if constexpr (sizeof(T) == 1) {
      return std::bit_cast<T>(read<uint8_t, BinaryEndianV>(Pos));
    }
    if constexpr (sizeof(T) == 2) {
      return std::bit_cast<T>(read<uint16_t, BinaryEndianV>(Pos));
    }
    if constexpr (sizeof(T) == 4) {
      return std::bit_cast<T>(read<uint32_t, BinaryEndianV>(Pos));
    }
    if constexpr (sizeof(T) == 8) {
      return std::bit_cast<T>(read<uint64_t, BinaryEndianV>(Pos));
    }
  }
  template <class T> T read(size_t Pos) const = delete;
  template <> constexpr byte_t read<byte_t>(size_t Pos) const {
    return std::bit_cast<byte_t>(read<uint8_t, std::endian::native>(Pos));
  }

  template <std::unsigned_integral T, std::endian BinaryEndianV>
  void write(size_t Pos, T Value) {
    if (Pos < Offset) [[unlikely]] {
      throw std::invalid_argument("Pos is lower than Offset.");
    }
    auto LocalPos = Pos - Offset;

    if ((LocalPos + sizeof(T)) >= CapacitySize) [[unlikely]] {
      throw std::range_error(
          "is (((Pos - Offset) + sizeof(T)) >= CapacitySize).");
    }

    if ((LocalPos + sizeof(T)) >= Size) {
      Size = Pos + sizeof(T);
    }

    if constexpr (sizeof(T) > 1) {
      if constexpr (BinaryEndianV != std::endian::native) {
        Value = std::byteswap(Value);
      }

      std::memcpy(Buffer.get() + LocalPos, &Value, sizeof(T));
      return;
    }

    Buffer[Pos] = std::bit_cast<value_type>(Value);
  }
  template <std::signed_integral T, std::endian BinaryEndianV>
  constexpr void write(size_t Pos, T Value) {
    static_assert(sizeof(T) <= 8, "Can conversion values.");
    if constexpr (sizeof(T) == 1) {
      write<uint8_t, BinaryEndianV>(Pos, std::bit_cast<uint8_t>(Value));
      return;
    }
    if constexpr (sizeof(T) == 2) {
      write<uint16_t, BinaryEndianV>(Pos, std::bit_cast<uint16_t>(Value));
      return;
    }
    if constexpr (sizeof(T) == 4) {
      write<uint32_t, BinaryEndianV>(Pos, std::bit_cast<uint32_t>(Value));
      return;
    }
    if constexpr (sizeof(T) == 8) {
      write<uint64_t, BinaryEndianV>(Pos, std::bit_cast<uint64_t>(Value));
      return;
    }
  }
  template <std::floating_point T, std::endian BinaryEndianV>
  constexpr void write(size_t Pos, T Value) {
    static_assert(sizeof(T) <= 8, "Can conversion values.");
    if constexpr (sizeof(T) == 1) {
      write<uint8_t, BinaryEndianV>(Pos, std::bit_cast<uint8_t>(Value));
      return;
    }
    if constexpr (sizeof(T) == 2) {
      write<uint16_t, BinaryEndianV>(Pos, std::bit_cast<uint16_t>(Value));
      return;
    }
    if constexpr (sizeof(T) == 4) {
      write<uint32_t, BinaryEndianV>(Pos, std::bit_cast<uint32_t>(Value));
      return;
    }
    if constexpr (sizeof(T) == 8) {
      write<uint64_t, BinaryEndianV>(Pos, std::bit_cast<uint64_t>(Value));
      return;
    }
  }
  template <class T> constexpr void write(size_t Pos, T Value) = delete;
  template <> constexpr void write<byte_t>(size_t Pos, byte_t Value) {
    write<uint8_t, std::endian::native>(Pos, std::bit_cast<uint8_t>(Value));
    return;
  }

  constexpr value_type &rawAccess(size_type Pos) {
    if (Pos >= CapacitySize) [[unlikely]] {
      throw std::out_of_range("Pos >= BufferSize");
    }
    return Buffer[Pos];
  }
  constexpr value_type const &rawAccess(size_type Pos) const {
    if (Pos >= CapacitySize) [[unlikely]] {
      throw std::out_of_range("Pos >= BufferSize");
    }
    return Buffer[Pos];
  }

  constexpr ret<std::reference_wrapper<value_type>>
  tryRawAccess(size_type Pos) noexcept {
    if (Pos >= CapacitySize) [[unlikely]] {
      return makeRetErr<ret<>::error_type>(
          ret<>::error_type::categoly_type::Logic,
          ret<>::error_type::descript_type::OutOfRange, "Pos >= BufferSize");
    }
    return std::ref(Buffer[Pos]);
  }
  constexpr ret<std::reference_wrapper<value_type const>>
  tryRawAccess(size_type Pos) const noexcept {
    if (Pos >= CapacitySize) [[unlikely]] {
      return makeRetErr<ret<>::error_type>(
          ret<>::error_type::categoly_type::Logic,
          ret<>::error_type::descript_type::OutOfRange, "Pos >= BufferSize");
    }
    return std::ref(Buffer[Pos]);
  }

  constexpr value_type &operator[](size_type Pos) noexcept {
    return Buffer[Pos];
  }
  constexpr value_type const &operator[](size_type Pos) const noexcept {
    return Buffer[Pos];
  }
};
} // namespace maboroutu
