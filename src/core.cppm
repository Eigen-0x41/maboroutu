module;
#include <cstdint>
export module maboroutu.core;

namespace maboroutu {
export template <typename T> struct basic_region {
   T offset;
   T size;
};

using region = basic_region<size_t>;

} // namespace maboroutu
