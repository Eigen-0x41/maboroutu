#pragma once

#include <algorithm>
#include <cstddef>
namespace maboroutu {

template <class T> void bytereverse(T *Val, size_t Size = 1) {
  std::reverse(reinterpret_cast<char *>(Val),
               reinterpret_cast<char *>(Val + Size));
}

} // namespace maboroutu
