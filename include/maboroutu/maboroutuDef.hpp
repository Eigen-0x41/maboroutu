#pragma once

#include <cstddef>
#include <vector>
namespace maboroutu {
using byte = typename std::byte;
using binary = typename std::vector<std::byte>;

static_assert(sizeof(typename binary::value_type) == 1,
              "Requested binary sizeof is not 1.");
} // namespace maboroutu
