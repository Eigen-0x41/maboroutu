#pragma once

#include "ExceptionHelper.hpp"
#include "listNode.hpp"

#include <concepts>
#include <stdexcept>
#include <variant>
#include <vector>

namespace maboroutu {

/**
 * @brief
 * 要素を自然数と0の範囲で索引付けして格納します。
 * 要素の挿入、削除、索引検索は全てO(1)で完了します。
 *
 * @tparam T [TODO:tparam]
 * @param Index [TODO:parameter]
 * @return [TODO:return]
 */
template <class T, std::integral IndexT = size_t> class SlotMap {
public:
  using this_type = SlotMap<T, IndexT>;

  using key_type = IndexT;
  using mapped_type = T;
  using unmapped_type = ListNode<this_type, IndexT>;

  using value_type = std::variant<mapped_type, unmapped_type>;

  static constexpr key_type MasterIndex = 0;

private:
  friend unmapped_type;

  std::vector<value_type> Continer;

  unmapped_type FreeNodeSentinel;

  unmapped_type &atValue(key_type Index) { return Continer.at(Index); }

public:
  SlotMap() : Continer({}), FreeNodeSentinel(this) {}
  SlotMap(this_type const &Value) = delete;
  SlotMap(this_type &&Value) = default;

  SlotMap(std::initializer_list<value_type> Value)
      : Continer(Value), FreeNodeSentinel(this) {}

  template <class... ArgsT> key_type emplace(ArgsT &&...Args) {
    auto &FreeIndex = FreeNodeSentinel.getForward();
    if (FreeNodeSentinel == FreeIndex) {
      key_type AllocIndex = Continer.size();
      Continer.emplace_back(std::in_place_type_t<mapped_type>(),
                            std::forward<ArgsT>(Args)...);
      return AllocIndex;
    }
    // deletorでリンク解除。
    key_type AllocIndex = FreeIndex.value();
    Continer.at(AllocIndex)
        .template emplace<mapped_type>(std::forward<ArgsT>(Args)...);
    return AllocIndex;
  }

  void erase(key_type Index) {
    auto &EraseNode = Continer.at(Index);
    if (std::holds_alternative<unmapped_type>(EraseNode))
      return;
    FreeNodeSentinel.insertHelper(EraseNode, Index, Index);
  }

  bool contains(key_type Index) const {
    if (Index >= Continer.size()) [[unlikely]] {
      return false;
    }

    return std::holds_alternative<mapped_type>(Continer[Index]);
  }

  /**
   * @brief
   * 要素を取得する。
   * もし要素が型TでなければruntimeErrorを排出する。
   *
   * @param Index [TODO:parameter]
   * @return [TODO:return]
   */
  mapped_type &at(key_type Index) {
    if (auto &RetValue = Continer.at(Index);
        std::holds_alternative<mapped_type>(RetValue)) [[likely]] {
      return std::get<mapped_type>(RetValue);
    }
    throw std::runtime_error(maboroutu::exceptionMessageCreater(
        this, "at", "Select index is not type template<T>!!"));
  }
  mapped_type &operator[](key_type Index) noexcept {
    return std::get<mapped_type>(Continer[Index]);
  }
};
} // namespace maboroutu
