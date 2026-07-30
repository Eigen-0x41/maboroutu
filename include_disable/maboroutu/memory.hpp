#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>

namespace maboroutu {

namespace concepts {

template <class T>
concept allocator = requires(T value) {
  typename T::value_type;
  typename T::size_type;

  { value.allocate(T::size_type(1)) } -> std::same_as<typename T::value_type *>;
  {
    value.deallocate(T::value_type * (nullptr))
  } -> std::same_as<typename T::value_type *>;
};
} // namespace concepts

} // namespace maboroutu
