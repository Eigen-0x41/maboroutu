#pragma once

#include <concepts>
#include <string>
#include <type_traits>
namespace maboroutu {
// referenced from <stdexcept>

enum class ExceptionCategoly : char {
  Logic,
  Runtime, // Is not can anticipate error.
};

enum class ExceptionDescript : char {
  None,
  Domain,
  Invalid_argument,
  Length,
  Out_of_range,
  Range,
  Overflow,
  Underflow,
};

template <class T>
concept ExceptionConcepts =
    requires(T &V) {
      { V.what() } -> std::same_as<typename T::string_type::value_type const *>;
      { V.strWhat() } -> std::same_as<typename T::const_string &>;
      { V.categoly() } -> std::same_as<typename T::categoly_type>;
      { V.descript() } -> std::same_as<typename T::descript_type>;
      { T::descript_type::None };
      { T::categoly_type::Logic };
      { T::categoly_type::Runtime };
    } && std::is_enum_v<typename T::categoly_type> &&
    std::is_enum_v<typename T::descript_type>;

template <class DescriptT = ExceptionDescript,
          class CategolyT = ExceptionCategoly>
class Exception {
private:
protected:
public:
  using this_type = Exception;

  using categoly_type = CategolyT;
  using descript_type = DescriptT;

  using string_type = std::string;
  using const_string = string_type const;

private:
  string_type Message;
  categoly_type Categoly;
  descript_type Descript;

protected:
public:
  [[deprecated]] Exception()
      : Message("[[Error message is not found.]]"),
        Categoly(categoly_type::Logic), Descript(descript_type::None) {}
  Exception(this_type const &) = default;
  Exception(this_type &&) = default;
  template <class... MessageT>
  Exception(categoly_type Categoly, MessageT &&...Message)
      : Message(std::forward<MessageT>(Message)...), Categoly(Categoly),
        Descript(descript_type::None) {}
  template <class... MessageT>
  Exception(categoly_type Categoly, descript_type Descript,
            MessageT &&...Message)
      : Message(std::forward<MessageT>(Message)...), Categoly(Categoly),
        Descript(Descript) {}
  ~Exception() = default;
  this_type &operator=(this_type const &) = default;
  this_type &operator=(this_type &&) = default;

  constexpr string_type::value_type const *what() const noexcept {
    return Message.c_str();
  }
  constexpr const_string &strWhat() const noexcept { return Message; }
  constexpr categoly_type categoly() const noexcept { return Categoly; }
  constexpr descript_type descript() const noexcept { return Descript; }
};
} // namespace maboroutu
