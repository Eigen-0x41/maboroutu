module;

#include <format>
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <string_view>
#include <utility>
export module maboroutu.console;

namespace maboroutu {

// [[command_hander]]
export class command_hander {
 public: /*STRUCT_FIELD*/
   using arg_type = std::string_view const;
   using args_type = std::span<arg_type>;

   using key_type = std::string_view const;
   struct mapped_type {
      using arg_type = arg_type;
      using args_type = args_type;

      std::function<int(command_hander &, args_type)> function;
      char const *description;
   };
   using value_type = std::map<key_type, mapped_type>;

 protected:
 private:
   using self_type = command_hander;

   value_type _value;
   args_type _first_args;
   /*--:  *IMPLIMENT_FIELD*/
 protected:
 public:
   command_hander() = delete;
   command_hander(command_hander const &) = delete;
   command_hander(command_hander &&) = delete;
   command_hander(value_type value, args_type args)
       : _value(std::move(value)), _first_args(args) {}
   ~command_hander() = default;

   auto value(this self_type const &self) noexcept -> value_type {
      return self._value;
   }

   auto run(this self_type &self, args_type args) -> std::optional<int> {
      for (size_t command_pos = 1; command_pos < args.size(); command_pos++) {
         if (args[command_pos].empty()) {
            continue;
         }
         if (args[command_pos].front() == '-') {
            continue;
         }
         if (!self._value.contains(args[command_pos])) {
            return std::nullopt;
         }
         return self._value[args[command_pos]].function(
             self, args.subspan(command_pos));
      }
      return std::nullopt;
   }

   auto operator=(command_hander const &rhs) -> command_hander & = default;
   auto operator=(command_hander &&rhs) -> command_hander & = default;
};

export using args_base_type = std::vector<std::string_view>;
export auto make_args_wrap(int argc, char *argv[]) -> args_base_type {
   return args_base_type{argv, argv + argc};
}

} // namespace maboroutu

using namespace maboroutu;
export template <>
struct std::formatter<command_hander> : std::formatter<const char *> {
   auto format(command_hander const &hander, std::format_context &ctx) const {
      auto out = ctx.out();
      out = std::format_to(out, "{: >22}\n", "commands");

#pragma unroll 4
      for (auto const &[name, entry] : hander.value()) {
         std::string_view sv = entry.description;
         auto line_end = sv.find('\n');
         out = std::format_to(out, "{}- {: >19} : {}\n",
                              (name.length() == 0) ? ' ' : name[0], name,
                              sv.substr(0, line_end));

#pragma unroll 2
         while (std::string_view::npos != line_end) {
            line_end++;
            if (line_end >= sv.size()) {
               return out;
            }
            sv = sv.substr(line_end);
            line_end = sv.find('\n');
            out =
                std::format_to(out, "{: >25}{}\n", "", sv.substr(0, line_end));
         }

         out = std::format_to(out, "\n");
      }
      return out;
   }
};
