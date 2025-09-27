#pragma once

#include <bit>
#include <climits>
#include <concepts>
namespace maboroutu {

template <class T>
concept BitAccessEntryConcepts =
    std::same_as<decltype(T::Size), const size_t> &&
    std::same_as<decltype(T::Begin), const size_t>;

template <std::unsigned_integral T> class BitAccess {
private:
  static constexpr size_t ValueBitSize = sizeof(T) * CHAR_BIT;

  template <size_t LBeginV, size_t SizeV> struct StaticAsserter {
    static_assert(SizeV > 0, "Requested field size is > 0.");
    static_assert((LBeginV + SizeV) <= ValueBitSize,
                  "Requested field is over than base type bit size.");
  };

protected:
public:
  using value_type = T;

  template <size_t LBeginV, size_t SizeV> struct Key {
    static constexpr size_t Begin = LBeginV;
    static constexpr size_t Size = SizeV;
  };

  template <size_t LBeginV, size_t SizeV>
  using main_key_t = Key<LBeginV, SizeV>;
  template <BitAccessEntryConcepts ForwardKeyT, size_t SizeV>
  using key_t = Key<ForwardKeyT::Begin + ForwardKeyT::Size, SizeV>;

  template <size_t LBeginV, size_t SizeV>
  class Accesser : StaticAsserter<LBeginV, SizeV> {
  public:
    using dependency_type = BitAccess;

    static constexpr size_t Begin = LBeginV;
    static constexpr size_t Size = SizeV;

  private:
    friend BitAccess;
    static constexpr value_type FilterBase = 0x1 << (Size - 0x1);
    static constexpr value_type Filter = FilterBase | (FilterBase - 0x1);
    static constexpr size_t ShiftSize = ValueBitSize - (Begin + Size);

    value_type &Container;

    Accesser(value_type &Value) : Container(Value) {}

  public:
    Accesser() = delete;
    Accesser(Accesser const &) = delete;
    Accesser(Accesser &&) = delete;

    Accesser &operator=(value_type Value) {
      Value = std::rotl<value_type>(Value & Filter, ShiftSize);

      Container = Container & ~std::rotl<value_type>(Filter, ShiftSize);

      Container = Container | Value;
      return *this;
    }
    Accesser &operator=(value_type Value) const = delete;

    operator value_type() const noexcept {
      return std::rotr(Container, ShiftSize) & Filter;
    }
  };

private:
  value_type Container;

protected:
public:
  BitAccess() = default;
  template <class TLoc>
  BitAccess(BitAccess<TLoc> const &Value) : Container(Value.Container) {}
  BitAccess(BitAccess &&Value) = default;
  BitAccess(value_type Value) : Container(Value) {}

  template <class TLoc>
  BitAccess &operator=(BitAccess<TLoc> const &Value) noexcept {
    Container = Value.Container;
    return *this;
  }
  BitAccess &operator=(BitAccess &&) noexcept = default;
  BitAccess &operator=(value_type Value) noexcept {
    Container = Value;
    return *this;
  }

  value_type &value() noexcept { return Container; }
  value_type const &value() const noexcept { return Container; }

  template <BitAccessEntryConcepts LocalT>
  Accesser<LocalT::Begin, LocalT::Size> operator()(LocalT Key) noexcept {
    return Accesser<LocalT::Begin, LocalT::Size>(Container);
  }

  template <BitAccessEntryConcepts LocalT>
  Accesser<LocalT::Begin, LocalT::Size> const &
  operator()(LocalT Key) const noexcept {
    return Accesser<LocalT::Begin, LocalT::Size>(Container);
  }
};
} // namespace maboroutu
