#pragma once

#include <array>
#include <bit>
#include <concepts>
#include <cstdint>
#include <fstream>
namespace maboroutu {
namespace byteio {

template <std::endian EndianT, std::integral T,
          std::derived_from<std::ifstream> IStreamDerived>
static inline T read(IStreamDerived &IFS) {
  static_assert(std::endian::native == std::endian::big ||
                    std::endian::native == std::endian::little,
                "native endian is big or little.");

  // C++ Standard guarantees that 1 == sizeof(char).
  std::array<char, sizeof(T)> BufArraydValue = {};
  IFS.read(BufArraydValue.data(), BufArraydValue.size());

  T BufValue = std::bit_cast<T>(BufArraydValue);
  if constexpr (EndianT != std::endian::native) {
    BufValue = std::byteswap(BufValue);
  }

  return BufValue;
}
template <std::endian EndianT, std::floating_point T,
          std::derived_from<std::ifstream> IStreamDerived>
static inline T read(IStreamDerived &IFS) {
  if constexpr (sizeof(T) == sizeof(uint32_t)) {
    return std::bit_cast<T>(read<EndianT, uint32_t, IStreamDerived>(IFS));
  }
  if constexpr (sizeof(T) == sizeof(uint64_t)) {
    return std::bit_cast<T>(read<EndianT, uint32_t, IStreamDerived>(IFS));
  } else {
    static_assert(true, "value is not support.");
  }
}

template <std::endian EndianT, std::integral T,
          std::derived_from<std::ifstream> OStreamDerived>
static inline void write(OStreamDerived &OFS, T Value) {
  static_assert(std::endian::native == std::endian::big ||
                    std::endian::native == std::endian::little,
                "native endian is big or little.");

  if constexpr (EndianT != std::endian::native) {
    Value = std::byteswap(Value);
  }
  // C++ Standard guarantees that 1 == sizeof(char).
  auto BufArraydValue = std::bit_cast<std::array<char, sizeof(T)>>(Value);
  OFS.write(BufArraydValue.data(), BufArraydValue.size());
}
template <std::endian EndianT, std::floating_point T,
          std::derived_from<std::ifstream> OStreamDerived>
static inline void write(OStreamDerived &OFS, T const Value) {
  if constexpr (sizeof(T) == sizeof(uint32_t)) {
    write<EndianT, uint32_t, OStreamDerived>(OFS,
                                             std::bit_cast<uint32_t>(Value));
    return;
  }
  if constexpr (sizeof(T) == sizeof(uint64_t)) {
    write<EndianT, uint32_t, OStreamDerived>(OFS,
                                             std::bit_cast<uint64_t>(Value));
    return;
  } else {
    static_assert(true, "value is not support.");
  }
}

} // namespace byteio
} // namespace maboroutu
