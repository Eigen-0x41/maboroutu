#pragma once

#include <array>
#include <cassert>
#include <deque>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace maboroutu {

namespace {
template <class DependT> class slot_map_key {
public:
  using size_type = size_t;

  static constexpr size_type npos = -1;

protected:
private:
  friend DependT;

  size_type _key;

  /*      IMPLIMENT_FIELD*/
  slot_map_key(size_type const &key) : _key(key) {}

protected:
public:
  slot_map_key() : _key(npos) {}
  slot_map_key(slot_map_key const &rhs) = default;
  slot_map_key(slot_map_key &&rhs) noexcept : _key(rhs._key) {}
  ~slot_map_key() = default;
  auto operator=(slot_map_key const &) -> slot_map_key & = default;
  auto operator=(slot_map_key &&) -> slot_map_key & = default;

  constexpr operator bool() const noexcept { return _key != npos; }
};

template <class DependT, class T> class slot_map_node {
public:
  using value_type = T;
  using key_type = slot_map_key<DependT>;

protected:
private:
  bool _is_enable;
  key_type _next;
  // 無名共用体
  value_type _value;
  /*      IMPLIMENT_FIELD*/

protected:
public:
  slot_map_node() : _is_enable(false), _next(), _value() {}
  auto operator=(const slot_map_node &) -> slot_map_node & = delete;
  auto operator=(slot_map_node &&) -> slot_map_node & = delete;
  slot_map_node(slot_map_node const &rhs)
      : _is_enable(rhs._is_enable), _next(rhs._next), _value(rhs._value) {}
  slot_map_node(slot_map_node &&rhs) noexcept
      : _is_enable(rhs._is_enable), _next(rhs._next),
        _value(std::move(rhs._value)) {}
  slot_map_node(value_type const &value)
      : _is_enable(true), _next(), _value(value) {}
  template <class... ArgsT>
  slot_map_node(ArgsT &&...args) : _is_enable(true), _next(), _value() {
    auto placeholder = _value;
    std::construct_at(&placeholder, std::forward<ArgsT>(args)...);
  }
  ~slot_map_node() {
    if (_is_enable) {
      destroy();
    }
  }

  constexpr auto next() noexcept -> key_type & { return _next; }
  constexpr auto next() const noexcept -> key_type const & { return _next; }

  constexpr auto value() noexcept -> value_type & {
    assert(_is_enable);
    return _value;
  }
  constexpr auto value() const noexcept -> value_type const & {
    assert(_is_enable);
    return _value;
  }

  template <class... ArgsT> constexpr void construct(ArgsT &&...args) {
    assert(!_is_enable);
    // ::new (static_cast<void *>(&Value.Value))
    // T(std::forward<ArgsT>(Args)...);
    std::construct_at(&_value, std::forward<ArgsT>(args)...);
    _is_enable = true;
  }
  constexpr void destroy() {
    assert(_is_enable);
    // &(Value.Value)->~T();
    std::destroy_at(&_value);
    _is_enable = false;
  }

  constexpr operator bool() const noexcept { return _is_enable; }
};

} // namespace

template <class T, class MakeContinerT> class basic_slot_map {
public: /*STRUCT_FIELD*/
  using key_type = slot_map_key<basic_slot_map>;
  using value_type = T;
  using size_type = size_t;

protected:
private:
  using node_type = slot_map_node<basic_slot_map, T>;
  using continer_type = MakeContinerT::template type<node_type>;

  continer_type _continer;
  key_type _next_constructed;
  key_type _next_destroyed;
  size_type _size = 0;
  size_type _free_size = 0;

  /*      IMPLIMENT_FIELD*/
protected:
public:
  basic_slot_map() : _continer(), _next_constructed(), _next_destroyed() {
    // 最適化されることを望む。
    if (_continer.size() != 0) {
      _free_size = _continer.size();
      for (size_t index = 0; index < _continer.size() - 1; ++index) {
        _continer[index].next() = index + 1;
      }

      _next_destroyed._key = 0;
    }
  }
  basic_slot_map(basic_slot_map const &rhs) = default;
  basic_slot_map(basic_slot_map &&rhs) = default;
  ~basic_slot_map() = default;
  auto operator=(basic_slot_map const &) -> basic_slot_map & = default;
  auto operator=(basic_slot_map &&) -> basic_slot_map & = default;

  auto contains(key_type const &key) const noexcept -> bool {
    if (!key) [[unlikely]] {
      return false;
    }
    if (key._key >= _continer.size()) [[unlikely]] {
      return false;
    }
    return bool(_continer[key._key]);
  }

  auto at(key_type const &key) -> value_type & {
    if (!contains(key)) [[unlikely]] {
      throw std::out_of_range("Key is not contains");
    }
    return _continer[key._key].value();
  }
  auto at(key_type const &key) const -> value_type const & {
    if (!contains(key)) [[unlikely]] {
      throw std::out_of_range("Key is not contains");
    }
    return _continer[key._key].value();
  }

  auto operator[](key_type const &key) noexcept -> value_type & {
    assert(contains(key));
    return _continer[key._key].value();
  }
  auto operator[](key_type const &key) const noexcept -> value_type const & {
    assert(contains(key));
    return _continer[key._key].value();
  }

  [[nodiscard]] auto size() const noexcept -> size_type { return _size; }
  [[nodiscard]] auto free_size() const noexcept -> size_type {
    return _free_size;
  }

  auto insert(value_type const &value) -> key_type {
    if (_next_destroyed) {
      key_type construct_target = _next_destroyed;
      node_type &target = _continer[construct_target._key];

      target.construct(value);
      _next_destroyed = target.next();

      target.next() = _next_constructed;
      _next_constructed = construct_target;

      --_free_size;
      ++_size;
      return construct_target;
    }

    if constexpr (requires() { _continer.push_back(node_type(value)); }) {

      key_type construct_target(_continer.size());
      _continer.push_back(node_type(value));
      _next_constructed = construct_target;

      ++_size;
      return construct_target;
    }
    throw std::out_of_range("_continer is not have push_back().");
  }
  template <class... ArgsT> auto emplace(ArgsT &&...args) -> key_type {
    if (_next_destroyed) {
      key_type construct_target = _next_destroyed;
      node_type &target = _continer[construct_target._key];

      target.construct(std::forward<ArgsT>(args)...);
      _next_destroyed = target.next();

      target.next() = _next_constructed;
      _next_constructed = construct_target;

      --_free_size;
      ++_size;
      return construct_target;
    }

    if constexpr (requires() {
                    _continer.emplace_back(std::forward<ArgsT>(args)...);
                  }) {
      key_type construct_target(_continer.size());
      _continer.emplace_back(std::forward<ArgsT>(args)...);
      _next_constructed = construct_target;

      ++_size;
      return construct_target;
    }
    throw std::out_of_range("_continer is not have emplace_back().");
  }
  auto erase(key_type const &key) -> key_type {
    if (!contains(key)) [[unlikely]] {
      throw std::out_of_range("Key is not contains.");
    }

    node_type &target = _continer[key._key];

    target.destroy();
    _next_constructed = target.next();

    target.next() = _next_destroyed;
    _next_destroyed = key;

    --_size;
    ++_free_size;

    return _next_constructed;
  }

  constexpr void shrink() {
    size_type delete_value_begin_index = _continer.size();
    while (delete_value_begin_index != 0) {
      --delete_value_begin_index;

      if (_continer[delete_value_begin_index]) {
        ++delete_value_begin_index;
        break;
      }
    }

    size_type const delete_size = _continer.size() - delete_value_begin_index;

    key_type current_key = _next_destroyed;
    for (auto i = 0; i < delete_size;) {
      auto &current_value = _continer[current_key];
      key_type const &target_key = current_value.Next;

      if (target_key >= delete_value_begin_index) {
        current_value.Next = _continer[target_key].Next;
        --i;
      }

      current_key = current_value.Next;
    }

    for (auto i = 0; i < delete_size; ++i) {
      _continer.pop_back();
    }

    _free_size -= delete_size;
  }

#if !defined(NDEBUG)

  [[deprecated("DEBUG FUNCTION")]] constexpr auto
  debug_f_get_key_index(key_type const &key) -> size_type {
    return key._key;
  }

#endif /*!defined(NDEBUG) */
};

struct make_deque {
  template <class T> using type = typename std::deque<T>;
};
struct make_vector {
  template <class T> using type = typename std::vector<T>;
};
template <size_t SizeV> struct make_array {
  template <class T> using type = typename std::array<T, SizeV>;
};

template <class T> using slot_map = basic_slot_map<T, make_deque>;
template <class T> using vector_slot_map = basic_slot_map<T, make_vector>;
template <class T, size_t SizeV>
using array_slot_map = basic_slot_map<T, make_array<SizeV>>;

} // namespace maboroutu
