module;
#include <cassert>
#include <deque>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>
export module maboroutu.slot_map;
import :node;

namespace maboroutu {

// [[basic_slot_map]]
export template <class IndexT, class T, class MakeContinerT>
   requires std::is_enum_v<IndexT>
class basic_slot_map {
 public: /*STRUCT_FIELD*/
   using index_type = IndexT;
   using value_type = T;
   using size_type = size_t;

   static const index_type npos = static_cast<index_type>(-1);

 protected:
 private:
   using self_type = basic_slot_map;

   using node_type = slot_map_node<basic_slot_map, T>;
   using continer_type = MakeContinerT::template type<node_type>;

   template <bool IsConst>
   // [[basic_iterator]]
   class basic_iterator {
    public: /*STRUCT_FIELD*/
      using difference_type = int;
      using value_type = typename std::pair<
          index_type const,
          typename std::conditional_t<IsConst, value_type const, value_type> &>;
      using iterator_concept = std::bidirectional_iterator_tag;

      class pointer {
       private:
         value_type _value;

       public:
         pointer(value_type value) : _value(value) {}
         auto operator*() -> value_type & { return _value; }
         auto operator->() -> value_type * { return &_value; }
      };

    protected:
    private:
      using self_type = basic_iterator;
      using depend_type =
          typename std::conditional_t<IsConst, basic_slot_map const,
                                      basic_slot_map>;

      friend basic_slot_map;

      depend_type *_data;
      size_type _idx_s;

      /*--:  *IMPLIMENT_FIELD*/
      basic_iterator(depend_type *data, size_type idx_s)
          : _data(data), _idx_s(idx_s) {}

    protected:
    public:
      basic_iterator() = default;
      basic_iterator(basic_iterator const &) = default;
      basic_iterator(basic_iterator &&) = default;
      ~basic_iterator() = default;

      friend constexpr auto operator++(self_type &self) -> self_type & {
         self._idx_s =
             (static_cast<size_type>(depend_type::npos) != self._idx_s)
                 ? self._data->_container[self._idx_s].next()
                 : self._data->_next_constructed;
         return self;
      }
      friend constexpr auto operator++(self_type &self, int) -> self_type {
         self_type ret_value = self;
         ++self;
         return ret_value;
      }

      friend constexpr auto operator--(self_type &self) -> self_type & {
         self._idx_s =
             (static_cast<size_type>(depend_type::npos) != self._idx_s)
                 ? self._data->_container[self._idx_s].prev()
                 : self._data->_rnext_constructed;
         return self;
      }
      friend constexpr auto operator--(self_type &self, int) -> self_type {
         self_type ret_value = self;
         --self;
         return ret_value;
      }

      friend constexpr auto operator*(self_type const &self) -> value_type {
         auto &data = self._data->_container[self._idx_s];
         assert(data.has_value());
         return std::make_pair(static_cast<index_type>(self._idx_s),
                               std::ref(data.value()));
      }

      constexpr auto operator->(this self_type const &self) -> pointer {
         return {*self};
      }

      friend constexpr auto operator==(self_type const &lhs,
                                       self_type const &rhs) -> bool {
         return (lhs._data == rhs._data) && (lhs._idx_s == rhs._idx_s);
      }

      auto operator=(basic_iterator const &rhs) -> basic_iterator & = default;
      auto operator=(basic_iterator &&rhs) -> basic_iterator & = default;
   };

   continer_type _container{};
   size_type _next_constructed = static_cast<size_type>(self_type::npos);
   size_type _rnext_constructed = static_cast<size_type>(self_type::npos);
   size_type _next_destroyed = static_cast<size_type>(self_type::npos);
   size_type _size = 0;
   size_type _free_size = 0;

   /*--:  *IMPLIMENT_FIELD*/
   static constexpr auto is_npos(size_type key) noexcept -> bool {
      return key == static_cast<size_type>(self_type::npos);
   }

 protected:
 public:
   using iterator = basic_iterator<false>;
   using const_iterator = basic_iterator<true>;

   basic_slot_map() {
      // 最適化されることを望む。
      if (_container.size() != 0) {
         _free_size = _container.size();
#pragma unroll 8
         for (auto &&[idx, node] :
              std::views::zip(std::views::iota(0), _container)) {
            node.prev() = idx - 1;
            node.next() = idx + 1;
         }

         _next_destroyed = 0;
      }
   }
   basic_slot_map(basic_slot_map const &) = default;
   basic_slot_map(basic_slot_map &&) = default;
   ~basic_slot_map() = default;

   [[nodiscard]] auto contains(this self_type const &self,
                               index_type const index) noexcept -> bool {
      auto idx = static_cast<size_type>(index);
      if (self_type::is_npos(idx)) [[unlikely]] {
         return false;
      }
      if (idx >= self._container.size()) [[unlikely]] {
         return false;
      }
      return bool(self._container[idx]);
   }

   auto at(this self_type &self, index_type const key) -> value_type & {
      if (!self.contains(key)) [[unlikely]] {
         throw std::out_of_range("Key is not contains");
      }
      return self._container[static_cast<size_type>(key)].value();
   }
   auto at(this self_type const &self, index_type const key)
       -> value_type const & {
      if (!self.contains(key)) [[unlikely]] {
         throw std::out_of_range("Key is not contains");
      }
      return self._container[static_cast<size_type>(key)].value();
   }

   auto operator[](this self_type &self, index_type const key) noexcept
       -> value_type & {
      assert(self.contains(key));
      return self._container[static_cast<size_type>(key)].value();
   }
   auto operator[](this self_type const &self, index_type const key) noexcept
       -> value_type const & {
      assert(self.contains(key));
      return self._container[static_cast<size_type>(key)].value();
   }

   [[nodiscard]] auto size(this self_type const &self) noexcept -> size_type {
      return self._size;
   }
   [[nodiscard]] auto free_size(this self_type const &self) noexcept
       -> size_type {
      return self._free_size;
   }

   [[nodiscard]] auto reserve(this self_type &self) -> index_type {
      if (!self_type::is_npos(self._next_destroyed)) {
         size_type construct_target = self._next_destroyed;
         node_type &target = self._container[construct_target];

         // target.construct(value);

         self._next_destroyed = target.next();
         target.prev() = static_cast<size_type>(self_type::npos);
         target.next() = static_cast<size_type>(self_type::npos);

         // self._next_constructed = construct_target;

         --self._free_size;
         // ++self._size;
         return static_cast<index_type>(construct_target);
      }

      if constexpr (requires() { self._container.push_back(node_type()); }) {

         size_type construct_target(self._container.size());
         self._container.push_back(node_type());
         self._next_constructed = construct_target;

         // ++self._size;
         return static_cast<index_type>(construct_target);
      }
      throw std::out_of_range("_container is not have push_back().");
   }
   /**
    * @brief insert value
    *
    * @param key reserved key
    * @param value construct value
    * @return if error then, return npos. else of return [@param key].
    */
   auto construct_at(this self_type &self, index_type key,
                     value_type const &value) -> index_type {
      if (self.contains(key)) [[unlikely]] {
         throw std::invalid_argument("Key is already constructed.");
      }

      node_type &target = self._container[static_cast<size_type>(key)];

      if (target.has_value()) [[unlikely]] {
         return self_type::npos;
      }

      target.construct(value);

      // construct linking
      target.prev() = static_cast<size_type>(self_type::npos);
      if (self.size() != 0) [[likely]] {
         auto &next = self._container[self._next_constructed];
         assert(self_type::is_npos(next.prev()));
         next.prev() = static_cast<size_type>(key);
      } else {
         self._rnext_constructed = static_cast<size_type>(key);
      }

      target.next() = self._next_constructed;
      self._next_constructed = static_cast<size_type>(key);

      ++self._size;
      return key;
   }
   /**
    * @brief emplace value
    *
    * @tparam ArgsT type for [@param args].
    * @tparam ArgsT reserved key
    * @param args construct value
    * @return if error then, return npos. else of return [@param key].
    */
   template <class... ArgsT>
   auto construct_at(this self_type &self, index_type key, ArgsT &&...args)
       -> index_type {
      if (self.contains(key)) [[unlikely]] {
         throw std::invalid_argument("Key is already constructed.");
      }
      node_type &target = self._container[static_cast<size_type>(key)];

      if (target.has_value()) [[unlikely]] {
         return self_type::npos;
      }

      target.construct(std::forward<ArgsT>(args)...);

      // construct linking
      target.prev() = static_cast<size_type>(self_type::npos);
      if (self.size() != 0) [[likely]] {
         auto &next = self._container[self._next_constructed];
         assert(self_type::is_npos(next.prev()));
         next.prev() = static_cast<size_type>(key);
      } else {
         self._rnext_constructed = static_cast<size_type>(key);
      }

      target.next() = self._next_constructed;
      self._next_constructed = static_cast<size_type>(key);

      ++self._size;
      return key;
   }

   auto insert(this self_type &self, value_type const &value) -> index_type {
      if (!self_type::is_npos(self._next_destroyed)) {
         size_type construct_target = self._next_destroyed;
         node_type &target = self._container[construct_target];

         target.construct(value);
         self._next_destroyed = target.next();

         // construct linking
         target.prev() = static_cast<size_type>(self_type::npos);
         if (self.size() != 0) [[likely]] {
            auto &next = self._container[self._next_constructed];
            assert(self_type::is_npos(next.prev()));
            next.prev() = construct_target;
         } else {
            self._rnext_constructed = construct_target;
         }

         target.next() = self._next_constructed;
         self._next_constructed = construct_target;

         --self._free_size;
         ++self._size;
         return static_cast<index_type>(construct_target);
      }

      if constexpr (requires() {
                       self._container.push_back(node_type(
                           {
                               .prev = static_cast<size_type>(self_type::npos),
                               .next = self._next_constructed,
                           },
                           value));
                    }) {
         size_type construct_target(self._container.size());
         self._container.push_back(node_type(
             {
                 .prev = static_cast<size_type>(self_type::npos),
                 .next = self._next_constructed,
             },
             value));
         if (self.size() != 0) [[likely]] {
            auto &next = self._container[self._next_constructed];
            assert(self_type::is_npos(next.prev()));
            next.prev() = construct_target;
         } else {
            self._rnext_constructed = construct_target;
         }
         self._next_constructed = construct_target;
         ++self._size;
         return static_cast<index_type>(construct_target);
      }
      throw std::out_of_range("_container is not have push_back().");
   }

   template <class... ArgsT>
   auto emplace(this self_type &self, ArgsT &&...args) -> index_type {
      if (!self_type::is_npos(self._next_destroyed)) {
         size_type construct_target = self._next_destroyed;
         node_type &target = self._container[construct_target];

         target.construct(std::forward<ArgsT>(args)...);
         self._next_destroyed = target.next();

         // construct linking
         target.prev() = static_cast<size_type>(self_type::npos);
         if (self.size() != 0) [[likely]] {
            auto &next = self._container[self._next_constructed];
            assert(self_type::is_npos(next.prev()));
            next.prev() = construct_target;
         } else {
            self._rnext_constructed = construct_target;
         }

         target.next() = self._next_constructed;
         self._next_constructed = construct_target;

         --self._free_size;
         ++self._size;
         return static_cast<index_type>(construct_target);
      }

      if constexpr (requires() {
                       self._container.emplace_back(
                           typename node_type::link{
                               .prev = static_cast<size_type>(self_type::npos),
                               .next = self._next_constructed,
                           },
                           std::forward<ArgsT>(args)...);
                    }) {
         size_type construct_target(self._container.size());
         self._container.push_back(node_type(
             {
                 .prev = static_cast<size_type>(self_type::npos),
                 .next = self._next_constructed,
             },
             std::forward<ArgsT>(args)...));
         if (self.size() != 0) [[likely]] {
            auto &next = self._container[self._next_constructed];
            assert(self_type::is_npos(next.prev()));
            next.prev() = construct_target;
         } else {
            self._rnext_constructed = construct_target;
         }
         self._next_constructed = construct_target;
         ++self._size;
         return static_cast<index_type>(construct_target);
      }
      throw std::out_of_range("_container is not have emplace_back().");
   }

   auto erase(this self_type &self, index_type const key) -> index_type {
      if (!self.contains(key)) [[unlikely]] {
         throw std::out_of_range("Key is not contains.");
      }

      node_type &target = self._container[static_cast<size_type>(key)];
      auto prev_idx = target.prev();

      target.destroy();
      ++self._free_size;
      --self._size;

      if (!self_type::is_npos(target.next())) [[likely]] {
         auto &next = self._container[target.next()];
         next.prev() = target.prev();
         if (!self_type::is_npos(target.prev())) [[likely]] {
            auto &prev = self._container[target.prev()];
            prev.next() = target.next();
         } else {
            self._next_constructed = target.next();
         }
      } else {
         self._rnext_constructed = target.prev();
         if (!self_type::is_npos(target.prev())) [[likely]] {
            auto &prev = self._container[target.prev()];
            prev.next() = static_cast<size_type>(self_type::npos);
         } else {
            self._next_constructed = static_cast<size_type>(self_type::npos);
         }
      }

      target.next() = self._next_destroyed;
      self._next_destroyed = static_cast<size_type>(key);
#if !defined(NDEBUG)
      // MEMO: この値は現状使用されていません。
      //       assert()にて、初期化されていることを確認するために使用しています。
      target.prev() = static_cast<size_type>(self_type::npos);
#endif /*!defined(NDEBUG) */

      return static_cast<index_type>(prev_idx);
   }

   constexpr void shrink(this self_type &self) {
      size_type delete_value_begin_index = self._container.size();
#pragma unroll 2
      while (delete_value_begin_index != 0) {
         --delete_value_begin_index;

         if (self._container[delete_value_begin_index]) {
            ++delete_value_begin_index;
            break;
         }
      }

      size_type const delete_size =
          self._container.size() - delete_value_begin_index;

      size_type current_key = self._next_destroyed;
#pragma unroll 2
      for (auto i = 0; i < delete_size;) {
         auto &current_value = self._container[current_key];
         size_type const &target_key = current_value.Next;

         if (target_key >= delete_value_begin_index) {
            current_value.Next = self._container[target_key].Next;
            --i;
         }

         current_key = current_value.Next;
      }

#pragma unroll 2
      for (auto i = 0; i < delete_size; ++i) {
         self._container.pop_back();
      }

      self._free_size -= delete_size;
   }

   constexpr auto begin(this self_type &self) noexcept -> iterator {
      return iterator(&self, self._next_constructed);
   }
   constexpr auto begin(this self_type const &self) noexcept -> const_iterator {
      return const_iterator(&self, self._next_constructed);
   }
   constexpr auto cbegin(this self_type const &self) noexcept
       -> const_iterator {
      return const_iterator(&self, self._next_constructed);
   }

   constexpr auto end(this self_type &self) noexcept -> iterator {
      return iterator(&self, static_cast<size_type>(self_type::npos));
   }
   constexpr auto end(this self_type const &self) noexcept -> const_iterator {
      return const_iterator(&self, static_cast<size_type>(self_type::npos));
   }
   constexpr auto cend(this self_type const &self) noexcept -> const_iterator {
      return const_iterator(&self, static_cast<size_type>(self_type::npos));
   }

   auto operator=(basic_slot_map const &rhs) -> basic_slot_map & = default;
   auto operator=(basic_slot_map &&rhs) -> basic_slot_map & = default;
};

export template <class IndexT, class T, class MakeContinerT>
auto get_if(basic_slot_map<IndexT, T, MakeContinerT> &slot_map,
            typename basic_slot_map<IndexT, T, MakeContinerT>::index_type index)
    -> basic_slot_map<IndexT, T, MakeContinerT>::value_type * {
   if (!slot_map.contains(index)) [[unlikely]] {
      return nullptr;
   }
   return &slot_map[index];
}
export template <class IndexT, class T, class MakeContinerT>
auto get_if(basic_slot_map<IndexT, T, MakeContinerT> const &slot_map,
            typename basic_slot_map<IndexT, T, MakeContinerT>::index_type index)
    -> basic_slot_map<IndexT, T, MakeContinerT>::value_type const * {
   if (!slot_map.contains(index)) [[unlikely]] {
      return nullptr;
   }
   return &slot_map[index];
}

struct make_deque {
   template <class T> using type = typename std::deque<T>;
};
template <size_t SizeV> struct make_array {
   template <class T> using type = typename std::array<T, SizeV>;
};

export template <class IndexT, class T>
using slot_map = basic_slot_map<IndexT, T, make_deque>;
export template <class IndexT, class T, size_t SizeV>
using inplace_slot_map = basic_slot_map<IndexT, T, make_array<SizeV>>;

} // namespace maboroutu
