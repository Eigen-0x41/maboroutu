#pragma once

#include <exception>
#include <expected>
#include <string>

namespace maboroutu {
template <class T>
using expected = typename std::expected<T, typename std::string>;
using unexpected = typename expected<void>::unexpected_type;

template <class T>
typename expected<T>::unexpected_type
unexpectedConv(expected<T> const &Expected) {
  return typename expected<T>::unexpected_type(Expected.error());
}
} // namespace maboroutu
