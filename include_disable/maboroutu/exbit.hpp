#pragma once

#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <span>
namespace maboroutu {

template <class T, size_t Extent> void bytereverse(std::span<T, Extent> span) {
  auto byte_span = std::as_writable_bytes(span);
  std::reverse(byte_span.begin(), byte_span.end());
}

// namespace {
// template <std::integral T, T V, T X, T Current, bool Status, bool IsSentinel>
// struct IsPowXyBase : public std::false_type {
//   static const size_t isLEThan = Current;
// };
//
// template <std::integral T, T V, T X, T Current>
// struct IsPowXyBase<T, V, X, Current, true, false> : public std::true_type {
//   static const size_t isLEThan = Current;
// };
//
// template <std::integral T, T V, T X, T Current>
// struct IsPowXyBase<T, V, X, Current, false, true>
//     : public IsPowXyBase<T, V, X, (Current * X), ((Current * X) == V),
//                          ((Current * X) < V)> {};
// } // namespace
//
// template <std::integral T, T V>
// struct IsPow2y : public IsPowXyBase<T, V, 2, 1, (1 == V), (1 < V)> {};
//
// template <std::integral T, T V, T X>
// struct IsPowXy : public IsPowXyBase<T, V, X, 1, (1 == V), (1 < V)> {};
// } // namespace maboroutu
//
// namespace maboroutu {
// namespace {
// template <std::integral T, T AlignKey> static inline T alignSizeBase(T Size)
// {
//   return (Size + AlignKey) & ~AlignKey;
// }
// } // namespace
//
// template <std::integral T, T Alignment> static inline T alignSize(T Size) {
//   static_assert(IsPow2y<T, Alignment>::value,
//                 "alignSize(): Alignment is Powers of 2.");
//   // 2^Xの倍数でない場合実質+(2^X)にし、
//   // /(2^X)することでアライメントをとっています。
//   // また、(2^X)は必ず一つのみビットが立っている状態であるため、
//   // 2^Xのビットフラグでフィルタリング
//   // することで除算の実行と同じ処理となります。
//   //
//   さらに、templateを使用して処理することで目的の処理内容を確実に実行します。
//   return alignSizeBase<T, Alignment - 1>(Size);
// }

template <std::integral T, size_t Alignment>
constexpr auto align_size_with_truncation(T value) noexcept -> T {
  static_assert(std::has_single_bit(Alignment),
                "alignSize(): Alignment is Powers of 2.");
  constexpr size_t align_key = Alignment - 1;
  return value & ~align_key;
}
template <std::integral T, size_t Alignment>
constexpr auto align_size(T value) noexcept -> T {
  static_assert(std::has_single_bit(Alignment),
                "alignSize(): Alignment is Powers of 2.");
  constexpr size_t align_key = Alignment - 1;
  return align_size_with_truncation<T, Alignment>(value + align_key);
}

static const bool is_mix_endian_v =
    (std::endian::native != std::endian::little) &&
    (std::endian::native != std::endian::big);

} // namespace maboroutu
