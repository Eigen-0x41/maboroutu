module;
#include <concepts>
export module maboroutu.link_node;

namespace maboroutu {

export template <class T, class NodeT>
concept link_nodable = requires(T &value) {
   { value.next } -> std::same_as<NodeT *&>;
};

export template <class T, class NodeT>
concept bidirectional_link_nodable = requires(T &value) {
   requires link_nodable<T, NodeT>;
   { value.prev } -> std::same_as<NodeT *&>;
};

export template <class T, class NodeT>
concept forward_link_nodable = requires(T &value) {
   requires !bidirectional_link_nodable<T, NodeT>;
   requires link_nodable<T, NodeT>;
};

template <class T, class Node>
concept link_node_member_accesser = requires(T const &accesser, Node &node) {
   { accesser(node) } -> link_nodable<Node>;
};

export template <class Accesser> struct link_node_traits {
 private:
   template <class T>
   static constexpr auto
   is_linked_bidirectional_node(bidirectional_link_nodable<T> auto &value)
       -> bool {
      return (value.next != nullptr) && (value.prev != nullptr);
   }
   template <class T>
   static constexpr auto
   is_linked_forward_node(bidirectional_link_nodable<T> auto &value) -> bool {
      return (value.next != nullptr) && (value.prev == nullptr);
   }
   template <class T>
   static constexpr auto
   is_linked_forward_node(forward_link_nodable<T> auto &value) -> bool {
      return value.next != nullptr;
   }

   template <class T>
   static constexpr auto
   is_unlinked_bidirectional_node(bidirectional_link_nodable<T> auto &value)
       -> bool {
      return (value.next == nullptr) && (value.prev == nullptr);
   }
   template <class T>
   static constexpr auto
   is_unlinked_forward_node(bidirectional_link_nodable<T> auto &value) -> bool {
      return (value.next == nullptr) && (value.prev == nullptr);
   }
   template <class T>
   static constexpr auto
   is_unlinked_forward_node(forward_link_nodable<T> auto &value) -> bool {
      return value.next == nullptr;
   }

 public:
   template <class T>
      requires link_node_member_accesser<Accesser, T>
   static constexpr auto insert(T &position, T &value) noexcept {
      assert(position != nullptr);
      assert(value != nullptr);

      Accesser accesser;

      bidirectional_link_nodable<T> auto &value_link = accesser(value);
      assert(is_unlinked_bidirectional_node<T>(value_link));

      bidirectional_link_nodable<T> auto &pos_link = accesser(position);
      bidirectional_link_nodable<T> auto &prev_link = accesser(pos_link.prev);
      assert(is_linked_bidirectional_node<T>(pos_link));
      assert(is_linked_bidirectional_node<T>(prev_link));

      value_link.next = pos_link;
      value_link.prev = prev_link;

      prev_link.next = value;
      pos_link.prev = value;

      assert(is_linked_bidirectional_node<T>(value_link));
      assert(is_linked_bidirectional_node<T>(pos_link));
      assert(is_linked_bidirectional_node<T>(prev_link));
   };

   template <class T>
      requires link_node_member_accesser<Accesser, T>
   static constexpr auto insert_after(T &position, T &value) noexcept {
      assert(position != nullptr);
      assert(value != nullptr);

      Accesser accesser;

      link_nodable<T> auto &value_link = accesser(value);
      assert(is_unlinked_forward_node<T>(value_link));

      link_nodable<T> auto &pos_link = accesser(position);
      link_nodable<T> auto &next_link = accesser(pos_link.next);
      assert(is_linked_forward_node<T>(pos_link));
      assert(is_linked_forward_node<T>(next_link));

      pos_link.next = value;
      value_link.next = next_link;

      next_link.prev = value;

      assert(is_linked_forward_node<T>(value_link));
      assert(is_linked_forward_node<T>(pos_link));
      assert(is_linked_forward_node<T>(next_link));
   };

   template <class T>
      requires link_node_member_accesser<Accesser, T>
   static constexpr auto erase(T &position) noexcept -> bool {
      assert(position != nullptr);

      Accesser accesser;

      bidirectional_link_nodable<T> auto &pos_link = accesser(position);
      assert(is_linked_bidirectional_node<T>(pos_link));

      if (pos_link.next == position) [[unlikely]] {
         return false;
      }

      bidirectional_link_nodable<T> auto &next_link = accesser(pos_link.next);
      bidirectional_link_nodable<T> auto &prev_node = accesser(pos_link.prev);
      assert(is_linked_bidirectional_node<T>(next_link));
      assert(is_linked_bidirectional_node<T>(prev_node));

      prev_node.next = next_link;
      next_link.prev = prev_node;

      pos_link.next = nullptr;
      pos_link.prev = nullptr;

      assert(is_unlinked_bidirectional_node<T>(pos_link));
      assert(is_linked_bidirectional_node<T>(next_link));
      assert(is_linked_bidirectional_node<T>(prev_node));

      return true;
   }

   template <class T>
      requires link_node_member_accesser<Accesser, T>
   static constexpr auto erase_after(T *position) noexcept -> bool {
      assert(position != nullptr);

      Accesser accesser;

      link_nodable<T> auto &pos_link = accesser(position);
      assert(is_linked_forward_node<T>(pos_link));

      if (pos_link->next == position) [[unlikely]] {
         return false;
      }

      link_nodable<T> auto &target_link = accesser(pos_link.next);
      link_nodable<T> auto &next_node = accesser(target_link.next);
      assert(is_linked_forward_node<T>(target_link));
      assert(is_linked_forward_node<T>(pos_link));

      pos_link.next = next_node;

      target_link.next = nullptr;

      assert(is_linked_forward_node<T>(pos_link));
      assert(is_unlinked_forward_node<T>(target_link));
      assert(is_linked_forward_node<T>(next_node));

      return true;
   }
};

} // namespace maboroutu
