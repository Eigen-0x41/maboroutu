module;
#include <cassert>
#include <variant>
export module maboroutu.slot_map:node;

namespace maboroutu {
// [[islot_map_node]]
template <class DependT, class T, class IIndex> class slot_map_node {
 public: /*STRUCT_FIELD*/
   using value_type = T;
   using iindex_type = IIndex;
   struct link {
      iindex_type prev;
      iindex_type next;
   };

 protected:
 private:
   link _link{
       .prev = static_cast<iindex_type>(DependT::npos),
       .next = static_cast<iindex_type>(DependT::npos),
   };
   std::variant<std::monostate, value_type> _value{};

   using self_type = slot_map_node;
   /*--:  *IMPLIMENT_FIELD*/
 protected:
 public:
   slot_map_node() = default;
   slot_map_node(slot_map_node const &rhs) = default;
   slot_map_node(slot_map_node &&rhs) noexcept = default;
   slot_map_node(link link, value_type const &value)
       : _link(link), _value(value) {}
   template <class... ArgsT>
   slot_map_node(link link, ArgsT &&...args)
       : _link(link),
         _value(std::in_place_type<value_type>, std::forward<ArgsT>(args)...) {}
   ~slot_map_node() = default;

   constexpr auto prev(this self_type &self) noexcept -> iindex_type & {
      return self._link.prev;
   }
   [[nodiscard]] constexpr auto prev(this self_type const &self) noexcept
       -> iindex_type const & {
      return self._link.prev;
   }
   constexpr auto next(this self_type &self) noexcept -> iindex_type & {
      return self._link.next;
   }
   [[nodiscard]] constexpr auto next(this self_type const &self) noexcept
       -> iindex_type const & {
      return self._link.next;
   }

   constexpr auto value(this self_type &self) noexcept -> value_type & {
      value_type *const ret_value = std::get_if<value_type>(&self._value);
      assert(ret_value != nullptr);
      return *ret_value;
   }
   constexpr auto value(this self_type const &self) noexcept
       -> value_type const & {
      value_type const *const ret_value = std::get_if<value_type>(&self._value);
      assert(ret_value != nullptr);
      return *ret_value;
   }

   template <class... ArgsT>
   constexpr void construct(this self_type &self, ArgsT &&...args) {
      assert(!self.has_value());
      self._value.template emplace<value_type>(std::forward<ArgsT>(args)...);
   }
   constexpr void destroy(this self_type &self) {
      assert(self.has_value());
      self._value.template emplace<std::monostate>();
   }

   [[nodiscard]] constexpr auto has_value(this self_type const &self) noexcept
       -> bool {
      return self._value.index() == 1;
   }

   constexpr operator bool(this self_type const &self) noexcept {
      return self.has_value();
   }

   auto operator=(slot_map_node const &rhs) -> slot_map_node & = default;
   auto operator=(slot_map_node &&rhs) -> slot_map_node & = default;
};
} // namespace maboroutu
