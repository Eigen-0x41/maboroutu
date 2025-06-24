#pragma once

#include <algorithm>

namespace maboroutu {

template <class T>
static inline void nativeByteReader(void const *Src, T *Dst,
                                    size_t const DstSize = 1) {
  T const *SrcCasted = reinterpret_cast<T const *>(Src);
  std::copy(SrcCasted, SrcCasted + DstSize, Dst);
}

template <class T>
static inline void reverseByteReader(void const *Src, T *Dst) {
  std::byte const *SrcCasted = reinterpret_cast<std::byte const *>(Src);
  std::byte *DstCasted = reinterpret_cast<std::byte *>(Dst);
  std::reverse_copy(SrcCasted, SrcCasted + sizeof(T), DstCasted);
}

template <class T>
static inline void reverseByteReader(void const *Src, T *Dst,
                                     size_t const DstSize) {
  for (size_t I = 0; I < DstSize; I++) {
    size_t const SrcOffset = sizeof(T) * I;
    reverseByteReader<T>(reinterpret_cast<std::byte const *>(Src) + SrcOffset,
                         Dst + I);
  }
}

template <class T>
static inline void nativeByteWriter(T const *Src, void *Dst,
                                    size_t const SrcSize = 1) {
  T *DstCasted = reinterpret_cast<T *>(Dst);
  std::copy(Src, Src + SrcSize, DstCasted);
}

template <class T>
static inline void reverseByteWriter(T const *Src, void *Dst) {
  std::byte const *SrcCasted = reinterpret_cast<std::byte const *>(Src);
  std::byte *DstCasted = reinterpret_cast<std::byte *>(Dst);
  std::reverse_copy(SrcCasted, SrcCasted + sizeof(T), DstCasted);
}

template <class T>
static inline void reverseByteWriter(T const *Src, void *Dst,
                                     size_t const SrcSize) {
  for (size_t I = 0; I < SrcSize; I++) {
    size_t const DstOffset = sizeof(T) * I;
    reverseByteWriter<T>(Src + I,
                         reinterpret_cast<std::byte *>(Dst) + DstOffset);
  }
}

template <class T>
static inline void reverseByte(T *Target, size_t const Size = 1) {
  std::reverse(reinterpret_cast<std::byte *>(Target),
               reinterpret_cast<std::byte *>(Target + Size));
}

} // namespace maboroutu
