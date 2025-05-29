#pragma once

#include "ExceptionHelper.hpp"
#include <concepts>
#include <limits>
#include <stdexcept>
#include <variant>
#include <vector>

namespace maboroutu {
namespace {
template <class DependT, class T, std::integral IndexT> class LinkListNode {
public:
  using this_type = LinkListNode<DependT, T, IndexT>;
  using dependency_pointer = DependT *;

  using index_type = IndexT;
  using value_type = T;

private:
  static constexpr index_type SentinelIndex =
      std::numeric_limits<index_type>::max();

  dependency_pointer Dependency;
  value_type Value;

  LinkListNode(dependency_pointer Dependency, index_type Forward,
               index_type Back, value_type &Value)
      : Dependency(Dependency), Forward(Forward), Back(Back), Value(Value) {};
  template <class... ArgsT>
  LinkListNode(dependency_pointer Dependency, index_type Forward,
               index_type Back, ArgsT &&...Args)
      : Dependency(Dependency), Forward(Forward), Back(Back),
        Value(std::forward<ArgsT>(Args)...){};

public:
  index_type Forward;
  index_type Back;

  LinkListNode() = delete;
  LinkListNode(this_type const &) = default;
  LinkListNode(this_type &&) = default;

  LinkListNode(dependency_pointer Dependency)
      : Dependency(Dependency), Forward(SentinelIndex), Back(SentinelIndex),
        Value() {}

  ~LinkListNode() {
    getForward().Back = Back;
    getBack().Forward = Forward;
  }

  index_type value() const noexcept { return Value; }
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
  void insertHelper(std::variant<Types...> &Variant, index_type Index,
                    value_type const &Value) noexcept {
    index_type &BackInForwardNode = getForward().Back;
    Variant = this_type(Dependency, Forward, BackInForwardNode, Value);
    Forward = Index;
    BackInForwardNode = Index;
  }
  template <class... Types, class... ArgsT>
  void emplaceHelper(std::variant<Types...> &Variant, index_type Index,
                     ArgsT &&...Args) noexcept {
    index_type &BackInForwardNode = getForward().Back;
    Variant = this_type(Dependency, Forward, BackInForwardNode,
                        std::forward<ArgsT>(Args)...);
    Forward = Index;
    BackInForwardNode = Index;
  }
};
} // namespace

// TODO:
// Array型なら何でも扱えるようにしたい。
template <class T, std::integral IndexT = size_t> class UnorderdIndexKeyMap {
public:
  using this_type = UnorderdIndexKeyMap<T, IndexT>;

  using index_type = IndexT;

  using mapped_type = T;
  using freeNodeList_type = LinkListNode<this_type, IndexT, IndexT>;
  using value_type = std::variant<mapped_type, freeNodeList_type>;

  static constexpr index_type MasterIndex = 0;

private:
  friend freeNodeList_type;

  std::vector<value_type> Continer;

  freeNodeList_type FreeNodeSentinel;

  freeNodeList_type &atValue(index_type Index) { return Continer.at(Index); }

public:
  UnorderdIndexKeyMap() : Continer({}), FreeNodeSentinel(this) {}
  UnorderdIndexKeyMap(this_type const &Value) = delete;
  UnorderdIndexKeyMap(this_type &&Value) = default;

  UnorderdIndexKeyMap(std::initializer_list<value_type> Value)
      : Continer(Value), FreeNodeSentinel(this) {}

  template <class... ArgsT> index_type emplace(ArgsT &&...Args) {
    auto &FreeIndex = FreeNodeSentinel.getForward();
    if (FreeNodeSentinel == FreeIndex) {
      index_type AllocIndex = Continer.size();
      Continer.emplace_back(std::in_place_type_t<mapped_type>(),
                            std::forward<ArgsT>(Args)...);
      return AllocIndex;
    }
    // deletorでリンク解除。
    index_type AllocIndex = FreeIndex.value();
    Continer.at(AllocIndex)
        .template emplace<mapped_type>(std::forward<ArgsT>(Args)...);
    return AllocIndex;
  }

  void erase(index_type Index) {
    FreeNodeSentinel.insertHelper(Continer.at(Index), Index, Index);
  }

  /**
   * @brief
   * 要素を取得する。
   * もし要素が型TでなければruntimeErrorを排出する。
   *
   * @param Index [TODO:parameter]
   * @return [TODO:return]
   */
  mapped_type &at(index_type Index) {
    if (auto &RetValue = Continer.at(Index);
        std::holds_alternative<mapped_type>(RetValue)) [[likely]] {
      return std::get<mapped_type>(RetValue);
    }
    throw std::runtime_error(maboroutu::exceptionMessageCreater(
        this, "at", "Select index is not type template<T>!!"));
  }
  mapped_type &operator[](index_type Index) noexcept {
    return std::get<mapped_type>(Continer.at(Index));
  }
};
} // namespace maboroutu
