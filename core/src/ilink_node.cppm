module;
#include <concepts>
export module maboroutu.ilink_node;

namespace maboroutu {

export template <class T>
concept ilink_nodable = requires(T &value) {
   { value.next } -> std::same_as<std::size_t &>;
};

export template <class T>
concept bidirectional_ilink_nodable = requires(T &value) {
   requires ilink_nodable<T>;
   { value.prev } -> std::same_as<std::size_t &>;
};

export template <class T>
concept forward_ilink_nodable = requires(T &value) {
   requires !bidirectional_ilink_nodable<T>;
   requires ilink_nodable<T>;
};

template <class T, class Container>
concept ilink_node_member_accesser =
    requires(T const &accesser, Container &node) {
       typename Container::size_type;
       requires std::integral<typename Container::size_type>;
       requires requires(Container::size_type index) {
          { accesser(node, index) } -> ilink_nodable;
       };
    };

export template <class Accesser> struct ilink_node_traits {
 private:
   template <class T>
   static constexpr auto
   is_linked_bidirectional_inode(bidirectional_ilink_nodable auto &value)
       -> bool {
      return (value.next != Accesser::npos) && (value.prev != Accesser::npos);
   }
   template <class T>
   static constexpr auto
   is_linked_forward_inole(bidirectional_ilink_nodable auto &value) -> bool {
      return (value.next != Accesser::npos) && (value.prev == Accesser::npos);
   }
   template <class T>
   static constexpr auto
   is_linked_forward_inode(forward_ilink_nodable auto &value) -> bool {
      return value.next != Accesser::npos;
   }

   template <class T>
   static constexpr auto
   is_unlinked_bidirectional_inode(bidirectional_ilink_nodable auto &value)
       -> bool {
      return (value.next == Accesser::npos) && (value.prev == Accesser::npos);
   }
   template <class T>
   static constexpr auto
   is_unlinked_forward_inode(bidirectional_ilink_nodable auto &value) -> bool {
      return (value.next == Accesser::npos) && (value.prev == Accesser::npos);
   }
   template <class T>
   static constexpr auto
   is_unlinked_forward_inode(forward_ilink_nodable auto &value) -> bool {
      return value.next == Accesser::npos;
   }

 public:
   template <class T>
      requires ilink_node_member_accesser<Accesser, T>
   static constexpr auto insert(T &container, T::size_type position,
                                T::size_type value) noexcept {
      assert(position != Accesser::npos);
      assert(value != Accesser::npos);

      Accesser accesser;

      bidirectional_ilink_nodable<T> auto &value_link =
          accesser(container, value);
      assert(is_unlinked_bidirectional_inode<T>(value_link));

      bidirectional_ilink_nodable<T> auto &pos_link =
          accesser(container, position);
      bidirectional_ilink_nodable<T> auto &prev_link =
          accesser(container, pos_link.prev);
      assert(is_linked_bidirectional_inode<T>(pos_link));
      assert(is_linked_bidirectional_inode<T>(prev_link));

      value_link.next = pos_link;
      value_link.prev = prev_link;

      prev_link.next = value;
      pos_link.prev = value;

      assert(is_linked_bidirectional_inode<T>(value_link));
      assert(is_linked_bidirectional_inode<T>(pos_link));
      assert(is_linked_bidirectional_inode<T>(prev_link));
   };

   template <class T>
      requires ilink_node_member_accesser<Accesser, T>
   static constexpr auto insert_after(T &container, T &position,
                                      T &value) noexcept {
      assert(position != Accesser::npos);
      assert(value != Accesser::npos);

      Accesser accesser;

      ilink_nodable<T> auto &value_link = accesser(container, value);
      assert(is_unlinked_forward_inode<T>(value_link));

      ilink_nodable<T> auto &pos_link = accesser(container, position);
      ilink_nodable<T> auto &next_link = accesser(container, pos_link.next);
      assert(is_linked_forward_inode<T>(pos_link));
      assert(is_linked_forward_inode<T>(next_link));

      pos_link.next = value;
      value_link.next = next_link;

      next_link.prev = value;

      assert(is_linked_forward_inode<T>(value_link));
      assert(is_linked_forward_inode<T>(pos_link));
      assert(is_linked_forward_inode<T>(next_link));
   };

   template <class T>
      requires ilink_node_member_accesser<Accesser, T>
   static constexpr auto erase(T &container, T &position) noexcept -> bool {
      assert(position != Accesser::npos);

      Accesser accesser;

      bidirectional_ilink_nodable<T> auto &pos_link =
          accesser(container, position);
      assert(is_linked_bidirectional_inode<T>(pos_link));

      if (pos_link.next == position) [[unlikely]] {
         return false;
      }

      bidirectional_ilink_nodable<T> auto &next_link =
          accesser(container, pos_link.next);
      bidirectional_ilink_nodable<T> auto &prev_node =
          accesser(container, pos_link.prev);
      assert(is_linked_bidirectional_inode<T>(next_link));
      assert(is_linked_bidirectional_inode<T>(prev_node));

      prev_node.next = next_link;
      next_link.prev = prev_node;

      pos_link.next = Accesser::npos;
      pos_link.prev = Accesser::npos;

      assert(is_unlinked_bidirectional_inode<T>(pos_link));
      assert(is_linked_bidirectional_inode<T>(next_link));
      assert(is_linked_bidirectional_inode<T>(prev_node));

      return true;
   }

   template <class T>
      requires ilink_node_member_accesser<Accesser, T>
   static constexpr auto erase_after(T &container, T &position) noexcept {
      assert(position != Accesser::npos);

      Accesser accesser;

      ilink_nodable<T> auto &pos_link = accesser(container, position);
      assert(is_linked_forward_inode<T>(pos_link));

      if (pos_link.next == position) [[unlikely]] {
         return false;
      }

      ilink_nodable<T> auto &target_link = accesser(container, pos_link.next);
      ilink_nodable<T> auto &next_node = accesser(container, target_link.next);
      assert(is_linked_forward_inode<T>(target_link));
      assert(is_linked_forward_inode<T>(pos_link));

      pos_link.next = next_node;

      target_link.next = Accesser::npos;

      assert(is_linked_forward_inode<T>(pos_link));
      assert(is_unlinked_forward_inode<T>(target_link));
      assert(is_linked_forward_inode<T>(next_node));

      return true;
   }
};

} // namespace maboroutu
