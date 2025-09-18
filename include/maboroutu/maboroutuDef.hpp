#pragma once

#include "maboroutu/error.hpp"
#include <cstddef>
#include <expected>
#include <type_traits>
#include <utility>
#include <vector>
namespace maboroutu {
using byte = typename std::byte;
using binary = typename std::vector<std::byte>;

static_assert(sizeof(typename binary::value_type) == 1,
              "Requested binary sizeof is 1.");

using exception = Exception<ExceptionDescript, ExceptionCategoly>;
template <class Ty = void, ExceptionConcepts Err = exception>
using ret = typename std::expected<Ty, Err>;
using retErr = typename std::unexpected<exception>;

template <ExceptionConcepts ErrT, class... ArgsT>
retErr makeRetErr(ArgsT &&...Args) {
  return retErr{ErrT(std::forward<ArgsT>(Args)...)};
}
template <class T> retErr convRetErr(ret<T> const &Ret) {
  return retErr{Ret.error()};
}

/**
 * @brief classによる参照型の実装。
 * CV修飾、pointer型を型Tに代入することはできません。
 *
 * @tparam T [TODO:tparam]
 * @return [TODO:return]
 */
template <class T> class VRef {
private:
protected:
public:
  using this_type = VRef;

  using value_type = T;
  static_assert(!std::is_const_v<value_type>, "Is not const.");
  static_assert(!std::is_volatile_v<value_type>, "Is not volatile.");
  static_assert(!std::is_pointer_v<value_type>, "Is not pointer.");

  using pointer = value_type *;
  using const_pointer = value_type const *;
  using reference = value_type &;
  using const_reference = value_type const &;

private:
  pointer Value;

protected:
public:
  VRef() = delete;
  VRef(this_type const &This) : Value(This.Value) {}
  VRef(this_type &&This) : Value(This.Value) {}
  VRef(reference Value) : Value(&Value) {}
  ~VRef() = default;
  this_type &operator=(this_type const &This) {
    *Value = *This.Value;
    return *this;
  }
  this_type &operator=(this_type &&This) {
    Value = This.Value;
    return *this;
  };
  this_type &operator=(const_reference V) noexcept {
    *Value = V;
    return *this;
  }
  operator reference() noexcept { return *Value; }
  auto operator*() noexcept -> decltype(**Value) { return **Value; }
  pointer operator->() noexcept { return Value; }

  friend bool operator==(this_type const &Lhs, this_type const &Rhs) noexcept {
    return *Lhs.Value == *Rhs.Value;
  }

  friend bool operator==(this_type const &Lhs, const_reference Rhs) noexcept {
    return *Lhs.Value == Rhs;
  }
};

} // namespace maboroutu
