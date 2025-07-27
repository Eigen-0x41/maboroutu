#pragma once

#include "maboroutu/maboroutuDef.hpp"
#include <cassert>
#include <deque>
#include <limits>
#include <optional>
#include <utility>
#include <variant>
namespace maboroutu {
/**
 * @brief
 * 実体を直接持つLinkList。
 * このクラスを実際に利用するには他のコンテナを利用する必要がある。
 *
 * 要件:
 * DependTにkey_typeが実装されていること。
 * DependTのvalue_typeがstd::variantであること。
 * T型がDependT::value_typeのテンプレート引数に含まれていること。
 *
 * @tparam DependT [TODO:tparam]
 * @tparam T [TODO:tparam]
 * @param Dependency [TODO:parameter]
 * @param Forward [TODO:parameter]
 * @param Back [TODO:parameter]
 * @param Value [TODO:parameter]
 * @return [TODO:return]
 */
template <class DependT, class T> class LinkNode {
public:
  using this_type = LinkNode<DependT, T>;
  using dependency_pointer = DependT *;

  using key_type = typename DependT::key_type;
  using value_type = T;

private:
  static constexpr key_type SentinelIndex =
      std::numeric_limits<key_type>::max();

  dependency_pointer Dependency;
  value_type Value;

  LinkNode(dependency_pointer Dependency, key_type Forward, key_type Back,
           value_type &Value)
      : Dependency(Dependency), Forward(Forward), Back(Back), Value(Value) {};
  template <class... ArgsT>
  LinkNode(dependency_pointer Dependency, key_type Forward, key_type Back,
           ArgsT &&...Args)
      : Dependency(Dependency), Forward(Forward), Back(Back),
        Value(std::forward<ArgsT>(Args)...){};

public:
  key_type Forward;
  key_type Back;

  LinkNode() = delete;
  LinkNode(this_type const &) = default;
  LinkNode(this_type &&) = default;

  LinkNode(dependency_pointer Dependency)
      : Dependency(Dependency), Forward(SentinelIndex), Back(SentinelIndex),
        Value() {}

  ~LinkNode() {
    getForward().Back = Back;
    getBack().Forward = Forward;
  }

  key_type value() const noexcept { return Value; }
  this_type &getForward() noexcept {
    if (Forward == SentinelIndex) [[unlikely]] {
      return Dependency->FreeNodeSentinel;
    }
    return std::get<this_type>(Dependency->Continer[Forward]);
  }
  this_type &getBack() noexcept {
    if (Back == SentinelIndex) [[unlikely]] {
      return Dependency->FreeNodeSentinel;
    }
    return std::get<this_type>(Dependency->Continer[Forward]);
  }

  this_type &operator=(this_type const &Value) = default;

  [[nodiscard]] friend bool operator==(this_type const &Left,
                                       this_type const &Right) noexcept {
    bool RetValue = true;
    RetValue &= (Left.Dependency == Right.Dependency);
    RetValue &= (Left.Forward == Right.Forward);
    RetValue &= (Left.Back == Right.Back);
    RetValue &= (Left.Value == Right.Value);

    return RetValue;
  }

  template <class... Types>
  void insertHelper(std::variant<Types...> &Variant, key_type Index,
                    value_type const &Value) noexcept {
    key_type &BackInForwardNode = getForward().Back;
    Variant = this_type(Dependency, Forward, BackInForwardNode, Value);
    Forward = Index;
    BackInForwardNode = Index;
  }
  template <class... Types, class... ArgsT>
  void emplaceHelper(std::variant<Types...> &Variant, key_type Index,
                     ArgsT &&...Args) noexcept {
    key_type &BackInForwardNode = getForward().Back;
    Variant = this_type(Dependency, Forward, BackInForwardNode,
                        std::forward<ArgsT>(Args)...);
    Forward = Index;
    BackInForwardNode = Index;
  }
};

/**
 * @brief
 * 直接リンクリストのノードを扱うためのクラス。
 * このクラスを実際に利用するには他のシーケンスコンテナを利用する必要がある。
 * また、特性上番兵にも値を保持しておくことができる。
 *
 * @tparam DependT [TODO:tparam]
 * @tparam T [TODO:tparam]
 * @param Depend [TODO:parameter]
 * @param Forward [TODO:parameter]
 * @param Back [TODO:parameter]
 * @param Args [TODO:parameter]
 * @return [TODO:return]
 */
template <class DependT, class T> class IndexedLinkNode {
private:
protected:
public:
  using this_type = IndexedLinkNode<DependT, T>;

  using dependency_type = DependT;
  using size_type = typename dependency_type::size_type;
  using value_type = T;

private:
  friend this_type;

  dependency_type &Depend;
  size_type Forward;
  size_type Back;
  value_type Value;

  template <class... ArgsT>
  IndexedLinkNode(dependency_type &Depend, size_type const Forward,
                  size_type const Back, ArgsT &&...Args)
      : Depend(Depend), Forward(Forward), Back(Back),
        Value(std::forward<ArgsT>(Args)...) {}

protected:
  size_t unlink() noexcept {
    std::optional<size_t> ThisIndex;

    if (Forward != NPos) [[likely]] {
      ThisIndex = Depend.Continer[Forward].Back;
      Depend.Continer[Forward].Back = Back;
    }
    if (Back != NPos) [[likely]] {
#ifdef _DEBUG
      if (ThisIndex) [[likely]] {
        assert(*ThisIndex == Depend.Continer[Back].Forward);
      }
#endif // _DEBUG
      Depend.Continer[Back].Forward = Forward;
    }

    Forward = NPos;
    Back = NPos;
    return (ThisIndex) ? *ThisIndex : NPos;
  }

public:
  static constexpr size_type NPos = -1;

  IndexedLinkNode() = delete;
  IndexedLinkNode(this_type const &) = default;
  IndexedLinkNode(this_type &&) = default;
  IndexedLinkNode(dependency_type &Depend)
      : Depend(Depend), Forward(NPos), Back(NPos) {}
  ~IndexedLinkNode() { unlink(); }
  this_type &operator=(this_type const &) = default;
  this_type &operator=(this_type &&) = default;
  this_type &operator=(value_type const &Arg) {
    Value = Arg;
    return *this;
  }

  this_type &insert(value_type const &Value) {
    // This node is BackNode.
    this_type &ForwardNode = Depend.Continer[Forward];
    auto const InsertIndex = Depend.Continer.size();

    Depend.Continer.emplace_back(Depend, Forward, ForwardNode.Back, Value);

    ForwardNode.Back = InsertIndex;
    Forward = InsertIndex;

    return *this;
  };
  template <class... ArgsT> this_type &emplace(ArgsT &&...Args) {
    auto &ForwardNode = Depend.Continer[Forward];

    auto const InsertIndex = Depend.Continer.size();
    Depend.Continer.emplace_back(Depend, Forward, ForwardNode.Back,
                                 std::forward<ArgsT>(Args)...);

    ForwardNode.Back = InsertIndex;
    Forward = InsertIndex;

    return *this;
  };
  ret<> move(size_type Place) noexcept {
    if (Place >= Depend.size()) [[unlikely]] {
      return makeRetErr(ret<>::error_type::categoly_type::Logic,
                        ret<>::error_type::descript_type::OutOfRange, "");
    }

    size_t const ThisIndex = unlink();

    this_type &BackNode = Depend[Place];
    this_type &ForwardNode = Depend[BackNode.Forward];

    Forward = BackNode.Forward;
    Back = ForwardNode.Back;

    ForwardNode.Back = ThisIndex;
    BackNode.Forward = ThisIndex;

    return {};
  }

  value_type &value() noexcept { return Value; }
  value_type const &value() const noexcept { return Value; }

  value_type &operator*() noexcept { return Value; }
  value_type const &operator*() const noexcept { return Value; }

  value_type *operator->() noexcept { return &Value; }
  value_type const *operator->() const noexcept { return &Value; }
};

template <class T, class ContinerT = std::deque<T>>
class IndexedLinkNodeContiner {
private:
protected:
public:
  using this_type = IndexedLinkNodeContiner<T, ContinerT>;

  using value_type = T;
  using continer_type = ContinerT;
  using size_type = size_t;

private:
  continer_type &Continer;
  value_type Sentinel;

protected:
public:
  static constexpr size_type NPos = -1;

  IndexedLinkNodeContiner() = default;
  /**
   * @brief Sentinelのコピーは行われません。
   */
  IndexedLinkNodeContiner(this_type const &V)
      : Continer(V.Continer), Sentinel(*this) {};
  IndexedLinkNodeContiner(this_type &&) = default;
  ~IndexedLinkNodeContiner() = default;
  /**
   * @brief Sentinelのコピーは行われません。
   *
   * @return [TODO:return]
   */
  this_type &operator=(this_type const &V) = delete;
  this_type &operator=(this_type &&) = default;

  value_type &operator[](size_t Pos) noexcept {
    if (Pos == NPos) [[unlikely]] {
      return Sentinel;
    }
    return Continer[Pos];
  }
  size_type size() const noexcept { return Continer.size(); }
};

} // namespace maboroutu
