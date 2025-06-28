#pragma once

#include <limits>
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
template <class DependT, class T> class ListNode {
public:
  using this_type = ListNode<DependT, T>;
  using dependency_pointer = DependT *;

  using key_type = typename DependT::key_type;
  using value_type = T;

private:
  static constexpr key_type SentinelIndex =
      std::numeric_limits<key_type>::max();

  dependency_pointer Dependency;
  value_type Value;

  ListNode(dependency_pointer Dependency, key_type Forward, key_type Back,
           value_type &Value)
      : Dependency(Dependency), Forward(Forward), Back(Back), Value(Value) {};
  template <class... ArgsT>
  ListNode(dependency_pointer Dependency, key_type Forward, key_type Back,
           ArgsT &&...Args)
      : Dependency(Dependency), Forward(Forward), Back(Back),
        Value(std::forward<ArgsT>(Args)...){};

public:
  key_type Forward;
  key_type Back;

  ListNode() = delete;
  ListNode(this_type const &) = default;
  ListNode(this_type &&) = default;

  ListNode(dependency_pointer Dependency)
      : Dependency(Dependency), Forward(SentinelIndex), Back(SentinelIndex),
        Value() {}

  ~ListNode() {
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
} // namespace maboroutu
