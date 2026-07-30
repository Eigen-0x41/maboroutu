#pragma once

#include <array>
#include <cassert>
#include <deque>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

namespace maboroutu {

namespace {
template <class DependT, class T> class islot_map_node {
public:
  using value_type = T;
  using size_type = size_t;

protected:
private:
  size_type _next = DependT::npos;
  // variantによる遅延構築
  std::variant<std::monostate, value_type> _value;
  /*      IMPLIMENT_FIELD*/

protected:
public:
  islot_map_node() : _value() {}
  islot_map_node(islot_map_node const &rhs)
      : _next(rhs._next), _value(rhs._value) {}
  islot_map_node(islot_map_node &&rhs) noexcept
      : _next(rhs._next), _value(std::move(rhs._value)) {}
  islot_map_node(value_type const &value) : _value(value) {}
  template <class... ArgsT>
  islot_map_node(ArgsT &&...args)
      : _value(std::in_place_type<value_type>, std::forward<ArgsT>(args)...) {}
  ~islot_map_node() = default;

  auto operator=(const islot_map_node &) -> islot_map_node & = delete;
  auto operator=(islot_map_node &&) -> islot_map_node & = delete;

  constexpr auto next() noexcept -> size_type & { return _next; }
  [[nodiscard]] constexpr auto next() const noexcept -> size_type const & {
    return _next;
  }

  constexpr auto value() noexcept -> value_type & {
    value_type *const ret_value = std::get_if<value_type>(&_value);
    assert(ret_value);
    return *ret_value;
  }
  constexpr auto value() const noexcept -> value_type const & {
    value_type *const ret_value = std::get_if<value_type>(&_value);
    assert(ret_value);
    return *ret_value;
  }

  template <class... ArgsT> constexpr void construct(ArgsT &&...args) {
    assert(!has_value());
    _value.template emplace<value_type>(std::forward<ArgsT>(args)...);
  }
  constexpr void destroy() {
    assert(has_value());
    _value.template emplace<std::monostate>();
  }

  [[nodiscard]] constexpr auto has_value() const noexcept -> bool {
    return _value.index() == 1;
  }

  constexpr operator bool() const noexcept { return has_value(); }
};
} // namespace

template <class T, class MakeContinerT> class basic_islot_map {
public: /*STRUCT_FIELD*/
  using value_type = T;
  using size_type = size_t;

  static const size_type npos = -1;

protected:
private:
  using node_type = islot_map_node<basic_islot_map, T>;
  using continer_type = MakeContinerT::template type<node_type>;

  continer_type _continer;
  size_type _next_constructed = npos;
  size_type _next_destroyed = npos;
  size_type _size = 0;
  size_type _free_size = 0;

  /*      IMPLIMENT_FIELD*/
  static auto is_npos(size_type key) noexcept -> bool { return key == npos; }

protected:
public:
  basic_islot_map() : _continer() {
    // 最適化されることを望む。
    if (_continer.size() != 0) {
      _free_size = _continer.size();
#pragma unroll 8
      for (size_t index = 0; index < _continer.size() - 1; ++index) {
        _continer[index].next() = index + 1;
      }

      _next_destroyed = 0;
    }
  }
  basic_islot_map(basic_islot_map const &rhs) = default;
  basic_islot_map(basic_islot_map &&rhs) = default;
  ~basic_islot_map() = default;
  auto operator=(basic_islot_map const &) -> basic_islot_map & = default;
  auto operator=(basic_islot_map &&) -> basic_islot_map & = default;

  [[nodiscard]] auto contains(size_type const key) const noexcept -> bool {
    if (is_npos(key)) [[unlikely]] {
      return false;
    }
    if (key >= _continer.size()) [[unlikely]] {
      return false;
    }
    return bool(_continer[key]);
  }

  auto at(size_type const key) -> value_type & {
    if (!contains(key)) [[unlikely]] {
      throw std::out_of_range("Key is not contains");
    }
    return _continer[key].value();
  }
  auto at(size_type const key) const -> value_type const & {
    if (!contains(key)) [[unlikely]] {
      throw std::out_of_range("Key is not contains");
    }
    return _continer[key].value();
  }

  auto operator[](size_type const key) noexcept -> value_type & {
    assert(contains(key));
    return _continer[key].value();
  }
  auto operator[](size_type const key) const noexcept -> value_type const & {
    assert(contains(key));
    return _continer[key].value();
  }

  [[nodiscard]] auto size() const noexcept -> size_type { return _size; }
  [[nodiscard]] auto free_size() const noexcept -> size_type {
    return _free_size;
  }

  auto insert(value_type const &value) -> size_type {
    if (!is_npos(_next_destroyed)) {
      size_type construct_target = _next_destroyed;
      node_type &target = _continer[construct_target];

      target.construct(value);
      _next_destroyed = target.next();

      target.next() = _next_constructed;
      _next_constructed = construct_target;

      --_free_size;
      ++_size;
      return construct_target;
    }

    if constexpr (requires() { _continer.push_back(node_type(value)); }) {

      size_type construct_target(_continer.size());
      _continer.push_back(node_type(value));
      _next_constructed = construct_target;

      ++_size;
      return construct_target;
    }
    throw std::out_of_range("_continer is not have push_back().");
  }
  template <class... ArgsT> auto emplace(ArgsT &&...args) -> size_type {
    if (!is_npos(_next_destroyed)) {
      size_type construct_target = _next_destroyed;
      node_type &target = _continer[construct_target];

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
      size_type construct_target(_continer.size());
      _continer.emplace_back(std::forward<ArgsT>(args)...);
      _next_constructed = construct_target;

      ++_size;
      return construct_target;
    }
    throw std::out_of_range("_continer is not have emplace_back().");
  }
  auto erase(size_type const key) -> size_type {
    if (!contains(key)) [[unlikely]] {
      throw std::out_of_range("Key is not contains.");
    }

    node_type &target = _continer[key];

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
#pragma unroll 2
    while (delete_value_begin_index != 0) {
      --delete_value_begin_index;

      if (_continer[delete_value_begin_index]) {
        ++delete_value_begin_index;
        break;
      }
    }

    size_type const delete_size = _continer.size() - delete_value_begin_index;

    size_type current_key = _next_destroyed;
#pragma unroll 2
    for (auto i = 0; i < delete_size;) {
      auto &current_value = _continer[current_key];
      size_type const &target_key = current_value.Next;

      if (target_key >= delete_value_begin_index) {
        current_value.Next = _continer[target_key].Next;
        --i;
      }

      current_key = current_value.Next;
    }

#pragma unroll 2
    for (auto i = 0; i < delete_size; ++i) {
      _continer.pop_back();
    }

    _free_size -= delete_size;
  }

#if !defined(NDEBUG)

  [[deprecated("DEBUG FUNCTION")]] constexpr auto
  debug_f_get_key_index(size_type const key) -> size_type {
    return key;
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

template <class T> using islot_map = basic_islot_map<T, make_deque>;
template <class T> using vector_islot_map = basic_islot_map<T, make_vector>;
template <class T, size_t SizeV>
using array_islot_map = basic_islot_map<T, make_array<SizeV>>;

} // namespace maboroutu
