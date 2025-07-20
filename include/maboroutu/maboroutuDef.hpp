#pragma once

#include "maboroutu/error.hpp"
#include <cstddef>
#include <expected>
#include <utility>
#include <vector>
namespace maboroutu {
using byte = typename std::byte;
using binary = typename std::vector<std::byte>;

static_assert(sizeof(typename binary::value_type) == 1,
              "Requested binary sizeof is not 1.");

template <class Ty = void, ExceptionConcepts Err =
                               Exception<ExceptionDescript, ExceptionCategoly>>
using ret = typename std::expected<Ty, Err>;
using retErr = typename ret<void>::unexpected_type;

template <ExceptionConcepts ErrT, class... ArgsT>
retErr makeRetErr(ArgsT &&...Args) {
  return retErr{ErrT(std::forward<ArgsT>(Args)...)};
}
template <class T> retErr convRetErr(ret<T> const &Ret) {
  return retErr{Ret.error()};
}

} // namespace maboroutu
