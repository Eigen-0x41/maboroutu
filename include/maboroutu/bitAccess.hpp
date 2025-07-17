#pragma once

#include <cmath>
#include <concepts>
#include <utility>
namespace maboroutu {

template <class T>
concept BitAccessEntryConcepts =
    std::same_as<decltype(T::Size), const size_t> &&
    std::same_as<decltype(T::Begin), const size_t> &&
    std::same_as<decltype(T::End), const size_t>;

template <std::unsigned_integral T> class BitAccess {
public:
  using this_type = BitAccess<T>;

  using value_type = T;

  template <size_t LBeginV, size_t SizeV> struct Key {
    static constexpr size_t Size = SizeV;
    static constexpr size_t Begin = LBeginV;
    static constexpr size_t End = Begin + Size;
  };

private:
  template <size_t LBeginV, size_t SizeV> struct StaticAsserter {
    static constexpr size_t ByteBitSize = 8;
    static constexpr size_t ValueBitSize = sizeof(value_type) * ByteBitSize;

    static_assert(SizeV > 0, "Requested field size is > 0.");
    static_assert((LBeginV + SizeV) <= ValueBitSize,
                  "Requested field is over than base type bit size.");
  };

  template <size_t LBeginV, size_t SizeV>
  struct KeyMakerBase : StaticAsserter<LBeginV, SizeV> {
    using value_type = Key<LBeginV, SizeV>;
  };

public:
  template <size_t LBeginV, size_t SizeV>
  struct KeyMaker : public KeyMakerBase<LBeginV, SizeV> {};
  template <BitAccessEntryConcepts ForwardKeyT, size_t SizeV>
  struct DependedKeyMaker : public KeyMakerBase<ForwardKeyT::End, SizeV> {};

  template <size_t LBeginV, size_t SizeV>
  class Accesser : StaticAsserter<LBeginV, SizeV> {
  public:
    using dependency_type = this_type;
    using this_type = Accesser<LBeginV, SizeV>;

    static constexpr size_t ValueBitSize =
        StaticAsserter<LBeginV, SizeV>::ValueBitSize;

    static_assert(SizeV > 0, "Requested field size is > 0.");
    static_assert((LBeginV + SizeV) <= ValueBitSize,
                  "Requested field is over than base type bit size.");

    static constexpr size_t Size = SizeV;
    static constexpr size_t Begin = LBeginV;
    static constexpr size_t End = Begin + Size;

  private:
    static constexpr value_type FilterBase = 0x1 << (Size - 0x1);
    static constexpr value_type Filter = FilterBase | (FilterBase - 0x1);
    static constexpr size_t ShiftSize = ValueBitSize - End;
    static constexpr value_type LShiftedFilter = Filter << ShiftSize;

    value_type &vValue() noexcept {
      return *reinterpret_cast<value_type *>(this);
    }
    value_type const &vValue() const noexcept {
      return *reinterpret_cast<value_type const *>(this);
    }

  public:
    Accesser() = delete;
    Accesser(Accesser const &) = delete;
    Accesser(Accesser &&) = delete;

    // Accesser(value_type &V) { this = reinterpret_cast<this_type *>(&V); }

    this_type &operator=(value_type Value) {
      Value = (Value << ShiftSize) & LShiftedFilter;

      value_type &Container = vValue();
      Container = Container & ~LShiftedFilter;

      Container = Container | Value;
      return *this;
    }
    operator value_type() const noexcept {
      value_type Container = vValue();
      return (Container >> ShiftSize) & Filter;
    }
  };

private:
  value_type Container;

public:
  template <BitAccessEntryConcepts LocalT>
  Accesser<LocalT::Begin, LocalT::Size> &operator()(LocalT Key) noexcept {
    return *reinterpret_cast<Accesser<LocalT::Begin, LocalT::Size> *>(
        &Container);
  }

  template <BitAccessEntryConcepts LocalT>
  Accesser<LocalT::Begin, LocalT::Size> const &
  operator()(LocalT Key) const noexcept {
    return *reinterpret_cast<Accesser<LocalT::Begin, LocalT::Size> *>(
        &Container);
  }

  value_type &operator*() noexcept { return Container; }
  value_type const &operator*() const noexcept { return Container; }
};
} // namespace maboroutu
