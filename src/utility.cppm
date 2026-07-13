module;
#include <bit>

export module maboroutu.utility;

namespace maboroutu {

export template <class EnumT, EnumT EnumV>
   requires std::is_enum_v<EnumT>
struct in_place_tag {
   using value_type = EnumT;
   static constexpr value_type value = EnumV;

   in_place_tag() = default;
   in_place_tag(in_place_tag const &) = default;
   in_place_tag(in_place_tag &&) = default;
};

export template <auto EnumV>
constexpr auto in_place_tag_v = in_place_tag<decltype(EnumV), EnumV>();

export template <std::endian EndianV>
using endian_in_place_tag = in_place_tag<std::endian, EndianV>;

} // namespace maboroutu
