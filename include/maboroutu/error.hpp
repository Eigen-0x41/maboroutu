#pragma once

#include <concepts>
#include <exception>
#include <expected>
#include <locale>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace maboroutu {

template <class T, class... Args> struct has_in_args {
  template <class Check, class... LocArgs>
  static consteval auto checker() noexcept -> bool {
    if constexpr (std::is_same_v<T, Check>) {
      return true;
    }
    if constexpr (requires {
                    { checker<LocArgs...>() };
                  }) {
      return checker<LocArgs...>();
    }
    return false;
  }

  static const bool value = checker<Args...>();
};

template <class... Args>
// [[error]]
class error_tag {
public: /*STRUCT_FIELD*/
protected:
private:
  std::variant<Args...> _tag;
  std::expected<std::string, char const *> _message;

  template <class LocT, class... LocArgs> struct args_picker {
    using value_type = LocT;
  };

  /*--:  *IMPLIMENT_FIELD*/
protected:
public:
  error_tag() = delete;
  error_tag(error_tag const &) = default;
  error_tag(error_tag &&) = default;
  template <class T>
  explicit error_tag(T /*unused*/, std::string const &message)
      : _tag(T()), _message(message) {
    error_tag::check_alternative<T>();
  }
  template <class T>
  explicit error_tag(T /*unused*/, char const *const message)
      : _tag(T()), _message(std::unexpected{message}) {
    error_tag::check_alternative<T>();
  }
  ~error_tag() = default;

  template <class T> static consteval auto has_alternative() noexcept -> bool {
    return has_in_args<T, Args...>::value;
  }

  template <class T> static consteval void check_alternative() noexcept {
    static_assert(has_alternative<T>(), "'this' is can hold requested type.");
  }

  [[nodiscard]] auto constexpr what() const noexcept -> char const *const {
    if (_message.has_value()) {
      return _message.value().c_str();
    }
    return _message.error();
  }

  auto operator=(error_tag const &rhs) -> error_tag & = default;
  auto operator=(error_tag &&rhs) -> error_tag & = default;

  template <class T>
  friend auto constexpr operator==(error_tag &left, T /*unused*/) -> bool {
    left.template check_alternative<T>();
    if (T *buf = std::get_if<T>(&left._tag)) {
      return true;
    }
    return false;
  }
  template <class T>
  friend auto constexpr operator==(T /*unused*/, error_tag const &right)
      -> bool {
    return right == T();
  }

  template <class T>
  static auto make_unexpected(T /*unused*/, std::string message) noexcept
      -> std::unexpected<error_tag> {
    return std::unexpected<error_tag>(std::in_place, T(), message);
  }
  template <class T>
  static auto make_unexpected(T /*unused*/, char const *const message) noexcept
      -> std::unexpected<error_tag> {
    return std::unexpected<error_tag>(std::in_place, T(), message);
  }
};
} // namespace maboroutu
