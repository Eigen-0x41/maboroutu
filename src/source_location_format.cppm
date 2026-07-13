module;
#include <format>
#include <source_location>
#include <print>

#define MABOROUTU_EXPECT_EQ(val, expect) if (val != expect) [[unlikely]]\
   std::println("{}\nvalue is not match. [ {}: {}, expect: {} ]", std::source_location::current(), \\\"val\\\\", val, expect)

export module maboroutu.source_location_format;

export template <>
struct std::formatter<std::source_location> : std::formatter<const char *> {
   static auto format(std::source_location srcloc, std::format_context &ctx) {
      return std::format_to(ctx.out(), "{}: {:3>}, {:3>}: {}",
                            srcloc.file_name(), srcloc.line(), srcloc.column(),
                            srcloc.function_name());
   }
};
