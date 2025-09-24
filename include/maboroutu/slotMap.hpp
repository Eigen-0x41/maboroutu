#pragma once

#include "maboroutu/maboroutuDef.hpp"
#include <cassert>
#include <deque>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

namespace maboroutu {

namespace {
// [[SlotMapKey]]
template <class DependT> class SlotMapKey {
private:
protected:
public:
  using this_type = SlotMapKey;

  using size_type = size_t;

  static constexpr size_type NPos = -1;

private:
  friend DependT;

  size_type Key;

  SlotMapKey(size_type const &Key) : Key(Key) {}

protected:
public:
  SlotMapKey() : Key(NPos) {}
  SlotMapKey(this_type const &Rhs) : Key(Rhs.Key) {}
  SlotMapKey(this_type &&Rhs) : Key(Rhs.Key) {}
  ~SlotMapKey() = default;
  this_type &operator=(this_type const &) = default;
  this_type &operator=(this_type &&) = default;

  constexpr operator bool() const noexcept { return Key != NPos; }
};

template <class DependT, class T> class SlotMapContiner {
public:
  using this_type = SlotMapContiner<DependT, T>;

  using value_type = T;
  using key_type = SlotMapKey<DependT>;

protected:
private:
private:
  bool IsEnable;
  key_type Next;
  // 無名共用体
  union {
    value_type Value;
  };

protected:
public:
  SlotMapContiner() : IsEnable(false), Next(), Value() {}
  SlotMapContiner(this_type const &Rhs)
      : IsEnable(Rhs.IsEnable), Next(Rhs.Next), Value(Rhs.Value) {}
  SlotMapContiner(this_type &&Rhs)
      : IsEnable(Rhs.IsEnable), Next(Rhs.Next), Value(std::move(Rhs.Value)) {}
  SlotMapContiner(value_type const &Value)
      : IsEnable(true), Next(), Value(Value) {}
  template <class... ArgsT>
  SlotMapContiner(ArgsT &&...Args) : IsEnable(true), Next(), Value() {
    std::construct_at(&Value, std::forward<ArgsT>(Args)...);
  }
  ~SlotMapContiner() {
    if (IsEnable) {
      destroy();
    }
  }

  constexpr key_type &next() noexcept { return Next; }
  constexpr key_type const &next() const noexcept { return Next; }

  constexpr value_type &value() noexcept {
    assert(IsEnable);
    return Value;
  }
  constexpr value_type const &value() const noexcept {
    assert(IsEnable);
    return Value;
  }

  template <class... ArgsT> constexpr void construct(ArgsT &&...Args) {
    assert(!IsEnable);
    // ::new (static_cast<void *>(&Value.Value))
    // T(std::forward<ArgsT>(Args)...);
    std::construct_at(&Value, std::forward<ArgsT>(Args)...);
    IsEnable = true;
  }
  constexpr void destroy() {
    assert(IsEnable);
    // &(Value.Value)->~T();
    std::destroy_at(&Value);
    IsEnable = false;
  }

  constexpr operator bool() const noexcept { return IsEnable; }
};

} // namespace

template <class T> class SlotMap {
public:
  using this_type = SlotMap<T>;

  using key_type = SlotMapKey<this_type>;
  using value_type = T;
  using size_type = size_t;

protected:
private:
  using continer_value = SlotMapContiner<this_type, T>;
  using continer = typename std::deque<continer_value>;

private:
  continer Continer;
  key_type NextConstructed;
  key_type NextDestroyed;
  size_type Size;
  size_type FreeSize;

protected:
public:
  SlotMap()
      : Continer(), NextConstructed(), NextDestroyed(), Size(0), FreeSize(0) {}
  SlotMap(this_type const &Rhs) = default;
  SlotMap(this_type &&Rhs) = default;
  ~SlotMap() = default;
  this_type &operator=(this_type const &) = default;
  this_type &operator=(this_type &&) = default;

  bool contains(key_type const &Key) const {
    if (!Key) [[unlikely]] {
      return false;
    }
    if (Key.Key >= Continer.size()) [[unlikely]] {
      return false;
    }
    return bool(Continer[Key.Key]);
  }

  value_type &at(key_type const &Key) noexcept {
    if (!contains(Key)) [[unlikely]] {
      throw std::out_of_range("Key is not contains");
    }
    return Continer[Key.Key].value();
  }
  value_type const &at(key_type const &Key) const noexcept {
    if (!contains(Key)) [[unlikely]] {
      throw std::out_of_range("Key is not contains");
    }
    return Continer[Key.Key].value();
  }

  ret<std::reference_wrapper<value_type>> tryAt(key_type const &Key) noexcept {
    if (!contains(Key)) [[unlikely]] {
      return makeRetErr<ret<>::error_type>(
          ret<>::error_type::categoly_type::Logic,
          ret<>::error_type::descript_type::OutOfRange, "Key is not contains.");
    }
    return std::ref(Continer[Key.Key].value());
  }
  ret<std::reference_wrapper<value_type> const>
  tryAt(key_type const &Key) const noexcept {
    if (!contains(Key)) [[unlikely]] {
      return makeRetErr<ret<>::error_type>(
          ret<>::error_type::categoly_type::Logic,
          ret<>::error_type::descript_type::OutOfRange, "Key is not contains.");
    }
    return std::ref(Continer[Key.Key].value());
  }

  value_type &operator[](key_type const &Key) noexcept {
    assert(contains(Key));
    return Continer[Key.Key].value();
  }
  value_type const &operator[](key_type const &Key) const noexcept {
    assert(contains(Key));
    return Continer[Key.Key].value();
  }

  size_type size() const noexcept { return Size; }
  size_type freeSize() const noexcept { return FreeSize; }

  key_type insert(value_type const &Value) {
    if (NextDestroyed) {
      key_type ConstructTarget = NextDestroyed;
      continer_value &Target = Continer[ConstructTarget.Key];

      Target.construct(Value);
      NextDestroyed = Target.next();

      Target.next() = NextConstructed;
      NextConstructed = ConstructTarget;

      --FreeSize;
      ++Size;
      return ConstructTarget;
    }

    key_type ConstructTarget(Continer.size());
    Continer.push_back(continer_value(Value));
    NextConstructed = ConstructTarget;

    ++Size;
    return ConstructTarget;
  }
  template <class... ArgsT> key_type emplace(ArgsT &&...Args) {
    if (NextDestroyed) {
      key_type ConstructTarget = NextDestroyed;
      continer_value &Target = Continer[ConstructTarget.Key];

      Target.construct(std::forward<ArgsT>(Args)...);
      NextDestroyed = Target.next();

      Target.next() = NextConstructed;
      NextConstructed = ConstructTarget;

      --FreeSize;
      ++Size;
      return ConstructTarget;
    }

    key_type ConstructTarget(Continer.size());
    Continer.emplace_back(std::forward<ArgsT>(Args)...);
    NextConstructed = ConstructTarget;

    ++Size;
    return ConstructTarget;
  }
  key_type erase(key_type const &Key) {
    if (!contains(Key)) [[unlikely]] {
      throw std::out_of_range("Key is not contains.");
    }

    continer_value &Target = Continer[Key.Key];

    Target.destroy();
    NextConstructed = Target.next();

    Target.next() = NextDestroyed;
    NextDestroyed = Key;

    --Size;
    ++FreeSize;

    return NextConstructed;
  }

  constexpr void shrink() {
    size_type DeleteValueBeginIndex = Continer.size();
    while (DeleteValueBeginIndex != 0) {
      --DeleteValueBeginIndex;

      if (Continer[DeleteValueBeginIndex]) {
        ++DeleteValueBeginIndex;
        break;
      }
    }

    size_type const DeleteSize = Continer.size() - DeleteValueBeginIndex;

    key_type CurrentKey = NextDestroyed;
    for (auto I = 0; I < DeleteSize;) {
      auto &CurrentValue = Continer[CurrentKey];
      key_type const &TargetKey = CurrentValue.Next;

      if (TargetKey >= DeleteValueBeginIndex) {
        CurrentValue.Next = Continer[TargetKey].Next;
        --I;
      }

      CurrentKey = CurrentValue.Next;
    }

    for (auto I = 0; I < DeleteSize; ++I) {
      Continer.pop_back();
    }

    FreeSize -= DeleteSize;
  }

#ifdef _DEBUG

  [[deprecated("DEBUG FUNCTION")]] constexpr size_type
  __DEBUG_F__GET_KEY_INDEX_(key_type const &Key) {
    return Key.Key;
  }

#endif // _DEBUG
};

} // namespace maboroutu
