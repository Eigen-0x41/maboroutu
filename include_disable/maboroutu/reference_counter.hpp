#pragma once

#include <cassert>
namespace maboroutu {

class reference_counter;
class reference_watcher;

namespace {
// [[reference_counter_continer]]
class reference_counter_continer {
public: /*STRUCT_FIELD*/
protected:
  friend reference_counter;
  friend reference_watcher;

private:
  long _owner = 1;
  long _watcher = 0; // watch only count.

  /*--:  *IMPLIMENT_FIELD*/
protected:
  reference_counter_continer() = default;
  ~reference_counter_continer() { assert((_owner + _watcher) == 0); }

  [[nodiscard]] constexpr auto owner() const noexcept -> long { return _owner; }
  [[nodiscard]] constexpr auto watcher() const noexcept -> long {
    return _watcher;
  }
  [[nodiscard]] constexpr auto all() const noexcept -> long {
    return _owner + _watcher;
  }

  [[nodiscard]] constexpr auto add_owner() noexcept
      -> reference_counter_continer & {
    _owner += 1;
    return *this;
  }
  [[nodiscard]] constexpr auto add_watcher() noexcept
      -> reference_counter_continer & {
    _watcher += 1;
    return *this;
  }

  constexpr void delete_owner() noexcept {
    _owner -= 1;
    assert(_owner > -1);
  }
  constexpr void delete_watcher() noexcept {
    _watcher -= 1;
    assert(_watcher > -1);
  }

public:
  reference_counter_continer(reference_counter_continer const &) = delete;
  reference_counter_continer(reference_counter_continer &&) = delete;

  auto operator=(reference_counter_continer const &rhs)
      -> reference_counter_continer & = delete;
  auto operator=(reference_counter_continer &&rhs)
      -> reference_counter_continer & = delete;
} __attribute__((aligned(sizeof(long))));
} // namespace

// [[reference_counter]]
class reference_counter {
public: /*STRUCT_FIELD*/
protected:
private:
  reference_counter_continer *_counter = nullptr;

  /*--:  *IMPLIMENT_FIELD*/
  constexpr void delete_helper() noexcept {
    if (_counter != nullptr) [[likely]] {
      _counter->delete_owner();
      if (_counter->all() == 0) [[unlikely]] {
        delete _counter;
      }
    }
  }

protected:
public:
  reference_counter() : _counter(new reference_counter_continer{}) {};
  reference_counter(reference_counter const &value)
      : reference_counter(value._counter) {}
  reference_counter(reference_counter &&value) noexcept
      : _counter(value._counter) {
    value._counter = nullptr;
  }
  reference_counter(reference_counter_continer *counter)
      : _counter(&counter->add_owner()) {}
  ~reference_counter() { delete_helper(); }

  operator reference_counter_continer *() noexcept { return _counter; }
  operator reference_counter_continer const *() const noexcept {
    return _counter;
  }

  auto operator=(reference_counter const &rhs) -> reference_counter & = default;
  auto operator=(reference_counter &&rhs) -> reference_counter & = default;
  auto operator=(reference_counter_continer *counter) noexcept
      -> reference_counter & {
    delete_helper();
    _counter = &counter->add_watcher();
    return *this;
  }

  constexpr void swap(reference_counter &value) noexcept {
    auto *const buffer = _counter;
    _counter = value._counter;
    value._counter = buffer;
  }

  void reset() noexcept { reference_counter().swap(*this); }
  void reset(reference_counter_continer *const counter) noexcept {
    delete_helper();
    _counter = &counter->add_owner();
  }

  [[nodiscard]] constexpr auto count() const noexcept -> long {
    return _counter->owner();
  }
  [[nodiscard]] constexpr auto unique() const noexcept -> bool {
    return count() == 1;
  }
};

// [[reference_watcher]]
class reference_watcher {
public: /*STRUCT_FIELD*/
protected:
private:
  reference_counter_continer *_counter = nullptr;

  /*--:  *IMPLIMENT_FIELD*/
  constexpr void delete_helper() noexcept {
    if (_counter != nullptr) [[likely]] {
      _counter->delete_watcher();
      if (_counter->all() == 0) [[unlikely]] {
        delete _counter;
      }
    }
  }

protected:
public:
  reference_watcher() = default;
  reference_watcher(reference_watcher const &value)
      : reference_watcher(value._counter) {}
  reference_watcher(reference_watcher &&value) noexcept
      : _counter(value._counter) {
    value._counter = nullptr;
  }
  reference_watcher(reference_counter_continer *counter) noexcept
      : _counter(&counter->add_watcher()) {}
  ~reference_watcher() { delete_helper(); }

  auto operator=(reference_watcher const &rhs) -> reference_watcher & = default;
  auto operator=(reference_watcher &&rhs) -> reference_watcher & = default;
  auto operator=(reference_counter_continer *counter) noexcept
      -> reference_watcher & {
    delete_helper();
    _counter = &counter->add_watcher();
    return *this;
  }

  operator reference_counter_continer *() noexcept { return _counter; }
  operator reference_counter_continer const *() const noexcept {
    return _counter;
  }

  constexpr void swap(reference_watcher &value) noexcept {
    auto *const buffer = _counter;
    _counter = value._counter;
    value._counter = buffer;
  }

  constexpr void reset() noexcept { reference_watcher().swap(*this); }

  [[nodiscard]] constexpr auto count() const noexcept -> long {
    if (_counter == nullptr) [[unlikely]] {
      return 0;
    }
    return _counter->owner();
  }
  [[nodiscard]] constexpr auto expired() const noexcept -> bool {
    return count() == 0;
  }
};
} // namespace maboroutu
