module;
#include <format>
#include <source_location>
export module maboroutu.source_location_format;

export template <>
struct std::formatter<std::source_location> : std::formatter<const char *> {
   static auto format(std::source_location srcloc, std::format_context &ctx) {
      return std::format_to(ctx.out(), "{}: {:3>}, {:3>}: {}",
                            srcloc.file_name(), srcloc.line(), srcloc.column(),
                            srcloc.function_name());
   }
};
