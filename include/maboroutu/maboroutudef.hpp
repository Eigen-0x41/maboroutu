#pragma once

#include <concepts>
#include <cstddef>
#include <exception>
#include <expected>
#include <vector>
namespace maboroutu {
using byte_t = typename std::byte;
using binary_t = typename std::vector<byte_t>;

template <class T>
using ret_type = typename std::expected<T, std::exception const *>;

template <class T, class... Args>
  requires std::constructible_from<T, Args...> &&
           std::derived_from<T, std::exception>
auto make_exception_error(Args &&...args) -> ret_type<void>::unexpected_type {
  static T holder(std::forward<Args>(args)...);
  return ret_type<void>::unexpected_type(std::in_place, &holder);
}
template <class T>
constexpr auto err_cast(T &value) -> ret_type<void>::unexpected_type {
  return ret_type<void>::unexpected_type(value.error());
}
} // namespace maboroutu
