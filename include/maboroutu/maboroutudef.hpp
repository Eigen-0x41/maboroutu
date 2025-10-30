#pragma once

#include "maboroutu/error.hpp"
#include <cstddef>
#include <expected>
#include <vector>
namespace maboroutu {
using byte_t = typename std::byte;
using binary_t = typename std::vector<byte_t>;

template <class Ty = void, exception_concepts Err = exception>
using ret_type = typename std::expected<Ty, Err>;

template <class T> struct make_ret_error_categoly {
  using type = typename T::error_type::categoly_type;
};
template <class T>
using make_ret_error_categoly_t = make_ret_error_categoly<T>::type;
template <class T> struct make_ret_error_descript {
  using type = typename T::error_type::descript_type;
};
template <class T>
using make_ret_error_descript_t = make_ret_error_descript<T>::type;

template <class ExpectedT, class... ArgsT>
auto make_ret_error(ArgsT &&...args) -> ExpectedT::unexprcted_type {
  return ExpectedT::unexpected_type(
      ExpectedT::error_type(std::forward<ArgsT>(args)...));
}

} // namespace maboroutu
