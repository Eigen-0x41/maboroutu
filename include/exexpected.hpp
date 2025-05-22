#pragma once

#include <exception>
#include <expected>
#include <string>

namespace maboroutu {
template <class T>
using expected = typename std::expected<T, typename std::string>;

template <class T, class ExpectedT = expected<T>>
typename ExpectedT::unexpected_type unexpectedConv(ExpectedT const &Expected) {
  return Expected.error();
}
} // namespace maboroutu
