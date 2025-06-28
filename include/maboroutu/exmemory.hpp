#pragma once

#include <concepts>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <new>
#include <type_traits>
namespace maboroutu {
struct free_delete {
  void operator()(void *Ptr) { free(Ptr); }
};
template <class T> struct operator_new_deletor {
  void operator()(T *Instance) {
    Instance->~T();
    ::operator delete(reinterpret_cast<void *>(Instance));
  }
};
struct fclose_delete {
  void operator()(FILE *File) { ::fclose(File); }
};
} // namespace maboroutu

namespace std {
/// 初期化なし。
// a_Size byte 分のメモリを確保。
template <class T>
unique_ptr<T, maboroutu::operator_new_deletor<T>>
make_unique_operator_new(size_t Size) {
  unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
  RetVal.reset(reinterpret_cast<T *>(::operator new(Size)));
  return RetVal;
}
// a_Size byte 分のメモリを a_Alignment の要件を満たして確保。
template <class T>
unique_ptr<T, maboroutu::operator_new_deletor<T>>
make_unique_operator_new(size_t Size, std::align_val_t Alignment) {
  unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
  RetVal.reset(reinterpret_cast<T *>(::operator new(Size, Alignment)));
  return RetVal;
}
// a_Size byte 分のメモリを確保。
//  リソースの確保は保証されない。
template <class T>
unique_ptr<T, maboroutu::operator_new_deletor<T>>
make_unique_operator_new(size_t Size, const std::nothrow_t &) {
  unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
  RetVal.reset(reinterpret_cast<T *>(::operator new(Size)));
  return RetVal;
}
// a_Size byte 分のメモリを a_Alignment の要件を満たして確保。
// リソースの確保は保証されない。
template <class T>
unique_ptr<T, maboroutu::operator_new_deletor<T>>
make_unique_operator_new(size_t Size, std::align_val_t Alignment,
                         const std::nothrow_t &) {
  unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
  RetVal.reset(reinterpret_cast<T *>(::operator new(Size, Alignment)));
  return RetVal;
}

/// 初期化あり。
// a_Size byte 分のメモリを確保。
template <class T, class... ArgsT>
unique_ptr<T, maboroutu::operator_new_deletor<T>>
make_unique_new(size_t Size, ArgsT &&...Args) {
  unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
  RetVal.reset(reinterpret_cast<T *>(::operator new(Size)));
  new (RetVal.get()) T(std::forward(Args)...);
  return RetVal;
}
// a_Size byte 分のメモリを a_Alignment の要件を満たして確保。
template <class T, class... ArgsT>
unique_ptr<T, maboroutu::operator_new_deletor<T>>
make_unique_new(size_t Size, std::align_val_t Alignment, ArgsT &&...Args) {
  unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
  RetVal.reset(reinterpret_cast<T *>(::operator new(Size, Alignment)));
  new (RetVal.get()) T(std::forward(Args)...);
  return RetVal;
}
// a_Size byte 分のメモリを確保。
// リソースの確保は保証されない。
template <class T, class... ArgsT>
unique_ptr<T, maboroutu::operator_new_deletor<T>>
make_unique_new(size_t Size, const std::nothrow_t &, ArgsT &&...Args) {
  unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
  RetVal.reset(reinterpret_cast<T *>(::operator new(Size)));
  new (RetVal.get()) T(std::forward(Args)...);
  return RetVal;
}
// a_Size byte 分のメモリを a_Alignment の要件を満たして確保。
// リソースの確保は保証されない。
template <class T, class... ArgsT>
unique_ptr<T, maboroutu::operator_new_deletor<T>>
make_unique_new(size_t Size, std::align_val_t Alignment, const std::nothrow_t &,
                ArgsT &&...Args) {
  unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
  RetVal.reset(reinterpret_cast<T *>(::operator new(Size, Alignment)));
  new (RetVal.get()) T(std::forward(Args)...);
  return RetVal;
}
} // namespace std

namespace maboroutu {
template <class T, class Deleter, class... ArgsT>
std::unique_ptr<T, Deleter>
restruct_unique(std::unique_ptr<T, Deleter> &&UniquePtr, ArgsT &&...Args) {
  UniquePtr.get()->~T();
  return new (UniquePtr.get()->~T()) T(Args...);
}
} // namespace maboroutu

namespace maboroutu {
namespace {
template <std::integral T, T V, T X, T Current, bool Status, bool IsSentinel>
struct IsPowXyBase : public std::false_type {
  static const size_t isLEThan = Current;
};

template <std::integral T, T V, T X, T Current>
struct IsPowXyBase<T, V, X, Current, true, false> : public std::true_type {
  static const size_t isLEThan = Current;
};

template <std::integral T, T V, T X, T Current>
struct IsPowXyBase<T, V, X, Current, false, true>
    : public IsPowXyBase<T, V, X, (Current * X), ((Current * X) == V),
                         ((Current * X) < V)> {};
} // namespace

template <std::integral T, T V>
struct IsPow2y : public IsPowXyBase<T, V, 2, 1, (1 == V), (1 < V)> {};

template <std::integral T, T V, T X>
struct IsPowXy : public IsPowXyBase<T, V, X, 1, (1 == V), (1 < V)> {};
} // namespace maboroutu

namespace maboroutu {
namespace {
template <std::integral T, T AlignKey> static inline T alignSizeBase(T Size) {
  return (Size + AlignKey) & ~AlignKey;
}
} // namespace

template <std::integral T, T Alignment> static inline T alignSize(T Size) {
  static_assert(IsPow2y<T, Alignment>::value,
                "alignSize(): Alignment is not Powers of 2!!");
  // 2^Xの倍数でない場合実質+(2^X)にし、
  // /(2^X)することでアライメントをとっています。
  // また、(2^X)は必ず一つのみビットが立っている状態であるため、
  // 2^Xのビットフラグでフィルタリング
  // することで除算の実行と同じ処理となります。
  // さらに、templateを使用して処理することで目的の処理内容を確実に実行します。
  return alignSizeBase<T, Alignment - 1>(Size);
}

} // namespace maboroutu

///  コンストラクタを呼ぶと値の初期化が入るため実装しません。
// template <class T, class... Args>
// unique_ptr<T, maboroutu::operator_new_deletor>
// make_unique_for_overwrite(size_t a_Size) {
//   unique_ptr<T, maboroutu::operator_new_deletor> r_Val =
//       reinterpret_cast<T *>(::operator new(a_Size));
//   return r_Val;
// }
