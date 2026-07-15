module;
#include <cassert>
#include <deque>
#include <stdexcept>
#include <type_traits>
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

   continer_type _continer{};
   size_type _next_constructed = static_cast<size_type>(npos);
   size_type _next_destroyed = static_cast<size_type>(npos);
   size_type _size = 0;
   size_type _free_size = 0;

   /*--:  *IMPLIMENT_FIELD*/
   static constexpr auto is_npos(size_type key) noexcept -> bool {
      return key == static_cast<size_type>(npos);
   }

 protected:
 public:
   basic_slot_map() {
      // 最適化されることを望む。
      if (_continer.size() != 0) {
         _free_size = _continer.size();
#pragma unroll 8
         for (size_t index = 0; index < _continer.size() - 1; ++index) {
            _continer[index].next() = index + 1;
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
      if (idx >= self._continer.size()) [[unlikely]] {
         return false;
      }
      return bool(self._continer[idx]);
   }

   auto at(this self_type &self, index_type const key) -> value_type & {
      if (!self.contains(key)) [[unlikely]] {
         throw std::out_of_range("Key is not contains");
      }
      return self._continer[static_cast<size_type>(key)].value();
   }
   auto at(this self_type const &self, index_type const key)
       -> value_type const & {
      if (!self.contains(key)) [[unlikely]] {
         throw std::out_of_range("Key is not contains");
      }
      return self._continer[static_cast<size_type>(key)].value();
   }

   auto operator[](this self_type &self, index_type const key) noexcept
       -> value_type & {
      assert(self.contains(key));
      return self._continer[static_cast<size_type>(key)].value();
   }
   auto operator[](this self_type const &self, index_type const key) noexcept
       -> value_type const & {
      assert(self.contains(key));
      return self._continer[static_cast<size_type>(key)].value();
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
         node_type &target = self._continer[construct_target];

         // target.construct(value);

         self._next_destroyed = target.next();
         target.next() = self_type::npos;

         // self._next_constructed = construct_target;

         --self._free_size;
         // ++self._size;
         return static_cast<index_type>(construct_target);
      }

      if constexpr (requires() { _continer.push_back(node_type()); }) {

         size_type construct_target(self._continer.size());
         self._continer.push_back(node_type());
         self._next_constructed = construct_target;

         // ++self._size;
         return static_cast<index_type>(construct_target);
      }
      throw std::out_of_range("_continer is not have push_back().");
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
      if (!self.contains(key)) [[unlikely]] {
         throw std::out_of_range("Key is not contains.");
      }
      node_type &target = self._continer[static_cast<size_type>(key)];

      if (target.has_value()) [[unlikely]] {
         return static_cast<size_type>(self_type::npos);
      }

      target.construct(value);

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
      if (!self.contains(key)) [[unlikely]] {
         throw std::out_of_range("Key is not contains.");
      }
      node_type &target = self._continer[static_cast<size_type>(key)];

      if (target.has_value()) [[unlikely]] {
         return static_cast<size_type>(self_type::npos);
      }

      target.construct(std::forward<ArgsT>(args)...);

      target.next() = self._next_constructed;

      self._next_constructed = static_cast<size_type>(key);

      ++self._size;
      return key;
   }

   auto insert(this self_type &self, value_type const &value) -> index_type {
      if (!self_type::is_npos(self._next_destroyed)) {
         size_type construct_target = self._next_destroyed;
         node_type &target = self._continer[construct_target];

         target.construct(value);
         self._next_destroyed = target.next();

         target.next() = self._next_constructed;
         self._next_constructed = construct_target;

         --self._free_size;
         ++self._size;
         return static_cast<index_type>(construct_target);
      }

      if constexpr (requires() { _continer.push_back(node_type(value)); }) {

         size_type construct_target(self._continer.size());
         self._continer.push_back(node_type(value));
         self._next_constructed = construct_target;

         ++self._size;
         return static_cast<index_type>(construct_target);
      }
      throw std::out_of_range("_continer is not have push_back().");
   }

   template <class... ArgsT>
   auto emplace(this self_type &self, ArgsT &&...args) -> index_type {
      if (!self_type::is_npos(self._next_destroyed)) {
         size_type construct_target = self._next_destroyed;
         node_type &target = self._continer[construct_target];

         target.construct(std::forward<ArgsT>(args)...);
         self._next_destroyed = target.next();

         target.next() = self._next_constructed;
         self._next_constructed = construct_target;

         --self._free_size;
         ++self._size;
         return static_cast<index_type>(construct_target);
      }

      if constexpr (requires() {
                       _continer.emplace_back(std::forward<ArgsT>(args)...);
                    }) {
         size_type construct_target(self._continer.size());
         self._continer.emplace_back(std::forward<ArgsT>(args)...);
         self._next_constructed = construct_target;

         ++self._size;
         return static_cast<index_type>(construct_target);
      }
      throw std::out_of_range("_continer is not have emplace_back().");
   }

   auto erase(this self_type &self, index_type const key) -> index_type {
      if (!self.contains(key)) [[unlikely]] {
         throw std::out_of_range("Key is not contains.");
      }

      node_type &target = self._continer[static_cast<size_type>(key)];

      target.destroy();
      self._next_constructed = target.next();

      target.next() = self._next_destroyed;
      self._next_destroyed = static_cast<size_type>(key);

      --self._size;
      ++self._free_size;

      return static_cast<index_type>(self._next_constructed);
   }

   constexpr void shrink(this self_type &self) {
      size_type delete_value_begin_index = self._continer.size();
#pragma unroll 2
      while (delete_value_begin_index != 0) {
         --delete_value_begin_index;

         if (self._continer[delete_value_begin_index]) {
            ++delete_value_begin_index;
            break;
         }
      }

      size_type const delete_size =
          self._continer.size() - delete_value_begin_index;

      size_type current_key = self._next_destroyed;
#pragma unroll 2
      for (auto i = 0; i < delete_size;) {
         auto &current_value = self._continer[current_key];
         size_type const &target_key = current_value.Next;

         if (target_key >= delete_value_begin_index) {
            current_value.Next = self._continer[target_key].Next;
            --i;
         }

         current_key = current_value.Next;
      }

#pragma unroll 2
      for (auto i = 0; i < delete_size; ++i) {
         self._continer.pop_back();
      }

      self._free_size -= delete_size;
   }

   auto operator=(basic_slot_map const &rhs) -> basic_slot_map & = default;
   auto operator=(basic_slot_map &&rhs) -> basic_slot_map & = default;
};

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
