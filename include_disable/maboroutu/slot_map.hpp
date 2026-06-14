#pragma once

#include "islot_map.hpp"
#include <print>
#include <utility>

namespace maboroutu {

namespace {
struct slot_map_key_access {
  using value_type = size_t;
  template <class T>
  static auto access(T const &target) noexcept -> value_type {
    return target._value;
  }
  template <class T>
  static auto access(T &target, value_type value) noexcept -> void {
    target._value = value;
  }
};
} // namespace

template <class T> struct slot_map_key : public T {
private:
  friend slot_map_key_access;
  using value_type = typename slot_map_key_access::value_type;
  value_type _value;
};

template <class Key, class T, class MakeContiner>
class basic_slot_map : private basic_islot_map<T, MakeContiner> {
  using base_type = basic_islot_map<T, MakeContiner>;

public: /*STRUCT_FIELD*/
  using key_type = Key;
  using value_type = typename base_type::value_type;
  using size_type = typename base_type::size_type;

protected:
private:
  /*--:  *IMPLIMENT_FIELD*/
  [[nodiscard]] static auto key_access(key_type const &key) noexcept
      -> slot_map_key_access::value_type {
    return slot_map_key_access::access(key);
  }
  static auto key_access(key_type &key, value_type value) noexcept -> void {
    slot_map_key_access::access(key, value);
  }

protected:
public:
  basic_slot_map() : base_type() {}
  basic_slot_map(basic_slot_map const &rhs) = delete;
  basic_slot_map(basic_slot_map &&rhs) noexcept : base_type(std::move(rhs)) {}
  ~basic_slot_map() = default;
  auto operator=(basic_slot_map const &) -> basic_slot_map & = delete;
  auto operator=(basic_slot_map &&) -> basic_slot_map & = default;

  [[nodiscard]] auto contains(key_type const &key) const noexcept -> bool {
    return base_type::contains(key_access(key));
  }

  auto at(key_type const &key) -> value_type & {
    return base_type::at(key_access(key));
  }
  auto at(key_type const &key) const -> value_type const & {
    return base_type::at(key_access(key));
  }

  auto operator[](key_type const &key) noexcept -> value_type & {
    return base_type::operator[](key_access(key));
  }
  auto operator[](size_type const &key) const noexcept -> value_type const & {
    return base_type::operator[](key_access(key));
  }

  [[nodiscard]] auto size() const noexcept -> size_type {
    return base_type::size();
  }
  [[nodiscard]] auto free_size() const noexcept -> size_type {
    return base_type::size();
  }

  auto insert(value_type const &value) -> key_type {
    key_type ret_value{};
    key_access(ret_value, base_type::insert(value));
    return ret_value;
  }
  template <class... Args> auto emplace(Args &&...args) -> key_type {
    key_type ret_value{};
    key_access(ret_value, base_type::emplace(std::forward<Args>(args)...));
    return ret_value;
  }
  auto erase(key_type const &key) -> key_type {
    key_type ret_value{};
    key_access(ret_value, base_type::erase(key_access(key)));
    return ret_value;
  }

  constexpr void shrink() { return base_type::shrink(); }

#if !defined(NDEBUG)

  [[deprecated("DEBUG FUNCTION")]] constexpr auto
  debug_f_get_key_index(key_type const &key) -> size_type {
    return key_access(key);
  }

#endif /*!defined(NDEBUG) */
};

template <class Key, class T>
using slot_map = basic_slot_map<Key, T, make_deque>;
template <class Key, class T>
using vector_slot_map = basic_slot_map<Key, T, make_vector>;
template <class Key, class T, size_t SizeV>
using array_slot_map = basic_slot_map<Key, T, make_array<SizeV>>;

} // namespace maboroutu
