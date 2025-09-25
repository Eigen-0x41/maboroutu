#pragma once

#include "maboroutu/error.hpp"
#include <climits>
#include <cstddef>
#include <expected>
#include <type_traits>
#include <utility>
#include <vector>
namespace maboroutu {
using byte_t = typename std::byte;
using binary_t = typename std::vector<byte_t>;

static_assert(sizeof(typename binary_t::value_type) == 1,
              "Requested binary sizeof is 1.");
static_assert(CHAR_BIT == 8, "Requested CHAR_BIT size is 8.");

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
template <class T> class VRef : public std::reference_wrapper<T> {
public:
  VRef() = delete;
  VRef(VRef const &This) = delete;
  VRef(VRef &&This) = delete;
  explicit VRef(std::reference_wrapper<T> Value)
      : std::reference_wrapper<T>::reference_wrapper(Value) {}
  ~VRef() = default;
};

} // namespace maboroutu

namespace std {
template <class T>
constexpr reference_wrapper<T> ref(maboroutu::VRef<T> &Value) noexcept {
  return ref(Value.get());
}
} // namespace std
