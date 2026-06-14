#pragma once

#include "./maboroutudef.hpp"
#include <type_traits>
namespace maboroutu {
template <class T, class ValueT, class... ArgsT>
concept io_concepts = requires(T &io, ValueT value) {
  requires std::is_constructible_v<T, binary_t &, ArgsT...>;
  requires std::is_constructible_v<T, binary_t &, size_t, ArgsT...>;
  { value = io };
  { io = value };
  { io.operator bool() };
};
} // namespace maboroutu
