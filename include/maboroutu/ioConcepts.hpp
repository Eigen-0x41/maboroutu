#pragma once

#include "./maboroutuDef.hpp"
#include <concepts>
#include <type_traits>
namespace maboroutu {
template <class T, class ValueT, class... ArgsT>
concept IOConcepts = requires(T &IO, ValueT Value) {
  requires std::is_constructible_v<T, binary &, ArgsT...>;
  requires std::is_constructible_v<T, binary &, size_t>;
  { Value = IO };
  { IO = Value };
};
} // namespace maboroutu
