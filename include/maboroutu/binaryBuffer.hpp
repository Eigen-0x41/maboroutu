#pragma once

#include "maboroutu/streamConcepts.hpp"
#include <bit>
#include <cassert>
#include <climits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <stdexcept>
namespace maboroutu {

class BinaryBuffer {
private:
protected:
public:
  using this_type = BinaryBuffer;
  using value_type = typename std::byte;
  using size_type = int64_t;

private:
  size_type Offset;
  size_type Size;
  std::unique_ptr<value_type[]> Buffer;
  size_type BufferSize;

  constexpr void rawWrite(value_type Value, size_type RawPos) {
    if (RawPos >= Size) [[unlikely]] {
      throw std::invalid_argument("");
    }
    Buffer[RawPos] = Value;
  }

  [[nodiscard]] constexpr value_type rawRead(size_type RawPos) {
    if (RawPos >= Size) [[unlikely]] {
      throw std::invalid_argument("");
    }
    return Buffer[RawPos];
  }

protected:
public:
  BinaryBuffer() : Offset(0), Size(0), Buffer(nullptr), BufferSize(0) {};
  BinaryBuffer(this_type const &Rhs) = delete;
  BinaryBuffer(this_type &&Rhs) = default;
  ~BinaryBuffer() {}
  this_type &operator=(this_type const &) = delete;
  this_type &operator=(this_type &&) = default;

  template <StreamIOConcept StIOT>
  void load(StIOT &Stream, size_type BufferSize) {
    load(Stream, BufferSize, Stream.fgetpos());
  }
  template <StreamIOConcept StIOT>
  void load(StIOT &Stream, size_type BufferSize, size_type Offset) {
    this->Offset = Offset;
    this->BufferSize = BufferSize;
    Buffer = std::make_unique<value_type[]>(BufferSize);
    Size = Stream.fread(Buffer.get(), sizeof(value_type),
                        BufferSize * sizeof(value_type));
  }
  template <StreamIOConcept StIOT>
  void store(StIOT &Stream, bool IsSyncPos = false) {
    if (IsSyncPos) {
      Stream.fsetpos(Offset);
    }
    Size = Stream.fwrite(Buffer.get(), sizeof(value_type),
                         BufferSize * sizeof(value_type));
  }

  template <std::unsigned_integral T, std::endian BinaryEndianV>
  T read(size_t Pos) {
    if (Pos < Offset) [[unlikely]] {
      throw std::invalid_argument("Pos is lower than Offset.");
    }
    auto RawPos = Pos - Offset;
    T RetValue = NULL;

    if ((RawPos + sizeof(T)) >= Size) [[unlikely]] {
      throw std::range_error("is ((Pos  + sizeof(T)) >= Size).");
    }

    for (auto I = 0; I < sizeof(T); I++) {
      RetValue |= std::rotl(static_cast<T>(rawRead(RawPos + I)),
                            CHAR_BIT * ((sizeof(T) - 1) - I));
    }

    if constexpr (BinaryEndianV == std::endian::big) {
      return RetValue;
    }
    return std::byteswap(RetValue);
  }
  template <std::signed_integral T, std::endian BinaryEndianV>
  T read(size_t Pos) {
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
  T read(size_t Pos) {
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

  template <std::unsigned_integral T, std::endian BinaryEndianV>
  void write(size_t Pos, T Value) {
    if (Pos < Offset) [[unlikely]] {
      throw std::invalid_argument("Pos is lower than Offset.");
    }
    auto LocalPos = Pos - Offset;

    if constexpr (BinaryEndianV == std::endian::little) {
      Value = std::byteswap(Value);
    }

    if ((LocalPos + sizeof(T)) >= Size) [[unlikely]] {
      throw std::range_error("is (((Pos - Offset) + sizeof(T)) >= Size).");
    }

    if ((LocalPos + sizeof(T)) >= BufferSize) {
      BufferSize = Pos + sizeof(T);
    }

    for (auto I = 0; I < sizeof(T); I++) {
      rawWrite(static_cast<std::byte>(
                   std::rotr(Value, CHAR_BIT * ((sizeof(T) - 1) - I))),
               I + LocalPos);
    }
  }
  template <std::signed_integral T, std::endian BinaryEndianV>
  void write(size_t Pos, T Value) {
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
  void write(size_t Pos, T Value) {
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
};
} // namespace maboroutu
