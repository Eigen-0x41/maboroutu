#pragma once

#include <expected>
#include <string>
#include <utility>
#include <variant>

namespace maboroutu {

template <class T, class... Args> struct has_in_args {
private:
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

public:
  static const bool value = checker<Args...>();
};

template <class... Args>
// [[error]]
class error_tag {
public: /*STRUCT_FIELD*/
  using value_type = typename std::variant<Args...>;

  template <class T = void> using result = std::expected<T, error_tag>;

protected:
private:
  std::expected<std::string, char const *> _message;
  value_type _tag;

  /*--:  *IMPLIMENT_FIELD*/
  template <class T> static consteval auto has_alternative() noexcept -> bool {
    return has_in_args<T, Args...>::value;
  }

  template <class T> static consteval void check_alternative() noexcept {
    static_assert(has_alternative<T>(), "'this' is can hold requested type.");
    static_assert(std::is_empty_v<T>, "T is empty class.");
  }

protected:
public:
  error_tag() = delete;
  error_tag(error_tag const &) = default;
  error_tag(error_tag &&) = default;
  template <class T>
  explicit error_tag(std::string const &message, T /*unused*/ = T())
      : _tag(T()), _message(message) {
    error_tag::check_alternative<T>();
  }
  template <class T>
  explicit error_tag(char const *const message, T /*unused*/ = T())
      : _tag(T()), _message(std::unexpected{message}) {
    error_tag::check_alternative<T>();
  }
  ~error_tag() = default;

  [[nodiscard]] auto constexpr what() const noexcept -> char const * {
    if (_message.has_value()) {
      return _message.value().c_str();
    }
    return _message.error();
  }

  [[nodiscard]] auto constexpr tag() const noexcept -> value_type const & {
    return _tag;
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
  static auto make_unexpected(std::string message) noexcept
      -> std::unexpected<error_tag> {
    return std::unexpected<error_tag>(std::in_place, message, T());
  }
  template <class T>
  static auto make_unexpected(char const *const message) noexcept
      -> std::unexpected<error_tag> {
    return std::unexpected<error_tag>(std::in_place, message, T());
  }
  template <class StrT, class... LocArgs>
  static auto make_unexpected(error_tag<LocArgs...> const &downward_compatible,
                              StrT message) noexcept
      -> std::unexpected<error_tag> {
    return std::visit(
        [&](auto tag) -> auto {
          using tag_type = decltype(tag);
          static_assert(has_alternative<tag_type>(),
                        "There is upward compatibility.");
          return std::unexpected<error_tag>(
              std::in_place, std::forward<StrT>(message), tag_type());
        },
        downward_compatible.tag());
  }
};
} // namespace maboroutu
