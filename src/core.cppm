module;
#include <cstdint>
#include <memory>
export module maboroutu.core;

namespace maboroutu {
export template <typename T> struct basic_region {
   T offset;
   T size;
};

using region = basic_region<size_t>;

export template <typename T> struct dynamic_array {
   std::unique_ptr<T[]> value;
   std::size_t size;
};

export using byte_array = dynamic_array<std::byte>;

} // namespace maboroutu
