#pragma once

#include "maboroutu/error.hpp"
#include <cstddef>
#include <expected>
#include <type_traits>
#include <utility>
#include <vector>
namespace maboroutu {
namespace {
template <class T> class MutableReference : public std::reference_wrapper<T> {
public:
  static_assert(!std::is_const_v<T>, "Value is not const.");

public:
  MutableReference() = delete;
  MutableReference(MutableReference const &This) = delete;
  MutableReference(MutableReference &&This) = delete;
  MutableReference(std::reference_wrapper<T> Value)
      : std::reference_wrapper<T>::reference_wrapper(Value) {}
  ~MutableReference() = default;
};
} // namespace

using byte_t = typename std::byte;
using binary_t = typename std::vector<byte_t>;

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

template <class T> using mut_ref_t = MutableReference<T>;
} // namespace maboroutu

namespace std {
template <class T>
constexpr reference_wrapper<T> ref(maboroutu::mut_ref_t<T> &Value) noexcept {
  return ref(Value.get());
}
} // namespace std
