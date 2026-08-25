module;
#include <array>
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include <variant>
export module maboroutu.extension_store;

namespace maboroutu {

template <class... Args> struct local_typeid {
 private:
   template <class T, class Cmp, class... LocArgs>
   static consteval auto cmp() -> size_t {
      if constexpr (!std::is_same_v<T, Cmp>) {
         return cmp<T, LocArgs...>();
      }
      return size() - sizeof...(LocArgs);
   }
   template <class T> static consteval auto cmp() -> size_t {
      static_assert(false, "is not entry type!");
   }

 public:
   static consteval auto size() -> size_t { return sizeof...(Args); }

   template <class T> static consteval auto value() -> size_t {
      return cmp<T, Args..., void>();
   }
   template <class T>
   static consteval auto value(T const & /*unused*/) -> size_t {
      return cmp<T, Args..., void>();
   }
};

// [[extension_store]]
export template <class... Types> class extension_store {
 public: /*STRUCT_FIELD*/
 protected:
 private:
   using self_type = extension_store;
   using id = local_typeid<Types...>;
   using value_type =
       std::array<std::variant<std::monostate, std::unique_ptr<Types>...>,
                  sizeof...(Types)>;

   static_assert(value_type::size() == id::size(), "is match size.");

   value_type _data{};

   /*--:  *IMPLIMENT_FIELD*/
 protected:
 public:
   extension_store() = default;
   extension_store(extension_store const &) = default;
   extension_store(extension_store &&) = default;
   ~extension_store() = default;

   template <class T, class... ArgsT>
      requires(std::same_as<T, Types> || ...)
   auto set(this self_type &self, ArgsT &&...args) -> void {
      self._data[id::template value<T>()].template emplace<std::unique_ptr<T>>(
          std::make_unique<T>(std::forward<ArgsT>(args)...));
   }

   template <class T>
      requires(std::same_as<T, Types> || ...)
   auto erase(this self_type &self) -> void {
      self._data[id::template value<T>()].template emplace<std::monostate>();
   }

   template <typename T>
      requires(std::same_as<T, Types> || ...)
   auto get_if(this self_type const &self) -> T * {
      constexpr auto idx = id::template value<T>();
      if (auto *uptr = std::get_if<std::unique_ptr<T>>(&self._data[idx]))
          [[likely]] {
         if (*uptr) [[likely]] {
            return uptr->get();
         }
      }
      return nullptr;
   }

   template <class Visitor>
      requires(std::invocable<Visitor &, Types &> && ...)
   auto visit_each(this self_type &self, Visitor &&vis) -> void {
      for (auto &slot : self._data) {
         std::visit(
             [&vis](auto &alt) {
                using alt_t = std::remove_cvref_t<decltype(alt)>;
                if constexpr (!std::is_same_v<alt_t, std::monostate>) {
                   if (alt) {
                      vis(*alt);
                   }
                }
             },
             slot);
      }
   }

   template <class Visitor>
      requires(std::invocable<Visitor &, const Types &> && ...)
   auto visit_each(this const self_type &self, Visitor &&vis) -> void {
      for (auto const &slot : self._data) {
         std::visit(
             [&vis](auto const &alt) {
                using alt_t = std::remove_cvref_t<decltype(alt)>;
                if constexpr (!std::is_same_v<alt_t, std::monostate>) {
                   if (alt) {
                      vis(*alt);
                   }
                }
             },
             slot);
      }
   }

   template <class Visitor>
      requires(std::invocable<Visitor &, const Types &> && ...)
   auto visit_each(this const self_type &self, Visitor &&vis) -> self_type {
      self_type ret_value;
      for (auto const &slot : self._data) {
         std::visit(
             [&ret_value, &vis](auto const &alt) {
                using alt_t = std::remove_cvref_t<decltype(alt)>;
                if constexpr (!std::is_same_v<alt_t, std::monostate>) {
                   if (alt) {
                      ret_value.template set<alt_t::element_type>(vis(*alt));
                   }
                }
             },
             slot);
      }
      return ret_value;
   }

   auto operator=(extension_store const &rhs) -> extension_store & = default;
   auto operator=(extension_store &&rhs) -> extension_store & = default;
};

} // namespace maboroutu
