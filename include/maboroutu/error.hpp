#pragma once

#include <concepts>
#include <string>
#include <type_traits>
namespace maboroutu {
// referenced from <stdexcept>

enum class exception_categoly : char {
  logic,
  runtime, // Is not can anticipate error.
};

enum class exception_descript : char {
  none,
  domain,
  invalid_argument,
  length,
  out_of_range,
  range,
  overflow,
  underflow,
};

template <class T>
concept exception_concepts = requires(T &value) {
  requires std::is_enum_v<typename T::categoly_type>;
  requires std::is_enum_v<typename T::descript_type>;
  T::descript_type::none;
  T::categoly_type::logic;
  T::categoly_type::runtime;
  { value.what() } -> std::same_as<typename T::string_type::value_type const *>;
  { value.str_what() } -> std::same_as<typename T::string_type const &>;
  { value.categoly() } -> std::same_as<typename T::categoly_type>;
  { value.descript() } -> std::same_as<typename T::descript_type>;
};

template <class CategolyT, class DescriptT> class basic_exception {
public: /*STRUCT_FIELD*/
  using categoly_type = CategolyT;
  using descript_type = DescriptT;

  using string_type = std::string;

protected:
private:
  string_type _message;
  categoly_type _categoly;
  descript_type _descript;

  /*      IMPLIMENT_FIELD*/
protected:
public:
  [[deprecated]] basic_exception()
      : _message("[[Error message is not found.]]"),
        _categoly(categoly_type::logic), _descript(descript_type::none) {}
  basic_exception(basic_exception const &) = default;
  basic_exception(basic_exception &&) = default;
  template <class... MessageT>
  basic_exception(categoly_type categoly, MessageT &&...message)
      : _message(std::forward<MessageT>(message)...), _categoly(categoly),
        _descript(descript_type::none) {}
  template <class... MessageT>
  basic_exception(categoly_type categoly, descript_type descript,
                  MessageT &&...message)
      : _message(std::forward<MessageT>(message)...), _categoly(categoly),
        _descript(descript) {}
  ~basic_exception() noexcept = default;

  auto operator=(basic_exception const &) -> basic_exception & = default;
  auto operator=(basic_exception &&) -> basic_exception & = default;

  [[nodiscard]] constexpr auto what() const noexcept
      -> string_type::value_type const * {
    return _message.c_str();
  }
  [[nodiscard]] constexpr auto str_what() const noexcept
      -> string_type const & {
    return _message;
  }
  [[nodiscard]] constexpr auto categoly() const noexcept -> categoly_type {
    return _categoly;
  }
  [[nodiscard]] constexpr auto descript() const noexcept -> descript_type {
    return _descript;
  }
};

using exception = basic_exception<exception_categoly, exception_descript>;
static_assert(std::destructible<exception>, "is destructible.");
static_assert(exception_concepts<exception>,
              "is maboroutu_exception_concepts.");

} // namespace maboroutu
