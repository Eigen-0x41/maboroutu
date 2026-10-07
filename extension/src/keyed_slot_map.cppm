module;
#include <cassert>
#include <expected>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
export module maboroutu.keyed_slot_map;
export import maboroutu.error;
import maboroutu.slot_map;

namespace maboroutu {

namespace errc {
enum class keyed_slot_map {
   failed_to_add_key,
   failed_to_constructed,
   key_was_not_contain,
   failed_to_domain_not_be_resolved,
};
}

export template <class T>
using keyed_slot_map_result = result<T, errc::keyed_slot_map>;

// [[keyed_slot_map]]
export template <class Key, class SlotMapT> class keyed_slot_map {
 public: /*STRUCT_FIELD*/
   // TODO: 命名については議論の余地あり。
   using key_type = Key;
   using mapped_type = std::remove_cvref_t<SlotMapT>;
   using domain_type =
       std::unordered_map<key_type, typename mapped_type::index_type>;

   static_assert(
       std::is_same_v<basic_slot_map<typename mapped_type::index_type,
                                     typename mapped_type::value_type,
                                     typename mapped_type::container_maker>,
                      mapped_type>,
       "Do not match type.");

 protected:
 private:
   using self_type = keyed_slot_map;

   using errc_type = keyed_slot_map_result<void>::error_type::code_type;

   mapped_type &_data;
   domain_type _domain;
   /*--:  *IMPLIMENT_FIELD*/
   template <class Self, class LocKeyT>
   [[nodiscard]] auto _find(this Self &self, LocKeyT const &key)
       -> decltype(auto) {
      // ms系のstlへの回避策
      if constexpr (requires { self._domain.find(key); }) {
         return self._domain.find(key);
      } else {
         return self._domain.find(typename domain_type::key_type(key));
      }
   }

   template <class Self, class LocKeyT>
   [[nodiscard]] auto _at(this Self &self, LocKeyT const &key)
       -> decltype(auto) {
      if (auto found = self._find(key); found != self._domain.end())
          [[likely]] {
         return found->second;
      }
      enter_fatal<std::out_of_range>("not contain the specified key");
   }

 protected:
 public:
   keyed_slot_map() = delete;
   keyed_slot_map(keyed_slot_map const &) = delete;
   keyed_slot_map(keyed_slot_map &&) = delete;
   keyed_slot_map(mapped_type &data) : _data(data), _domain() {}
   ~keyed_slot_map() = default;

   template <class Self>
   [[deprecated("Please avoid using this function.")]] auto
   container(this Self &self)
       -> std::conditional_t<std::is_const_v<Self>, mapped_type const,
                             mapped_type> & {
      return self._data;
   }

   /**
    * @brief bypass function
    * @note
    * 前提: require_routed_existが成功値であること
    * keyが存在している場合、そのkeyに対応するindexより実体が取得できる。
    * keyが存在していない場合は致命的エラーとなる。例外が有効な場合は
    * std::out_of_rangeを送出し、無効な場合はプログラムを終了する。
    */
   template <class Self, class LocKeyT>
   auto routed_at(this Self &self, LocKeyT const &key) -> decltype(auto) {
      auto const index = self._at(key);
      // slot_mapのkeyは明示的確保のみ対応。
      if constexpr (std::is_const_v<Self>) {
         assert(std::as_const(self._data).contains(index));
         return std::as_const(self._data)[index];
      } else {
         assert(self._data.contains(index));
         return self._data[index];
      }
   }
   /**
    * @brief bypass function
    * @note
    * 非常に強力な動作を行えるため、使用は最小限に留めること。
    * keyが存在している場合、funcが実行される。
    * keyが存在していない場合は致命的エラーとなる。例外が有効な場合は
    * std::out_of_rangeを送出し、無効な場合はプログラムを終了する。
    */
   template <class Self, class Func, class LocKeyT>
   auto routed_access(this Self &self, Func &&func, LocKeyT const &key)
       -> decltype(auto) {
      if constexpr (std::is_const_v<Self>) {
         return std::invoke(std::forward<Func>(func), std::as_const(self._data),
                            self._at(key));
      } else {
         return std::invoke(std::forward<Func>(func), self._data,
                            self._at(key));
      }
   }

   /**
    * @brief keyを登録し、対応する要素を構築する。
    * @note
    * 例外が有効な場合のみ、次の失敗を戻り値へ変換して元の状態へ戻す。
    * - keyの登録が例外を送出した場合: failed_to_add_key
    * - 要素のコンストラクタが例外を送出した場合: failed_to_constructed
    * 例外が無効な場合、これらの失敗は発生しない。
    * 登録済みのkeyと重複した場合は、例外の有無にかかわらず
    * failed_to_add_keyを返す。
    */
   template <class Self, class LocKeyT, class... Args>
   auto routed_emplace(this Self &self, LocKeyT &&key, Args &&...args)
       -> keyed_slot_map_result<void> {
      auto const index = self._data.checkout();
      std::pair<typename domain_type::iterator, bool> ret_value{};
      auto registered = invoke_or_recover(
          [&]() -> keyed_slot_map_result<void> {
             ret_value =
                 self._domain.emplace(std::forward<LocKeyT>(key), index);
             return {};
          },
          [&]() -> keyed_slot_map_result<void> {
             self._data.cancel(index);
             return make_unexpected(errc_type::failed_to_add_key);
          });
      if (!registered) [[unlikely]] {
         return registered;
      }
      if (!ret_value.second) [[unlikely]] {
         self._data.cancel(index);
         return make_unexpected(errc_type::failed_to_add_key);
      }
      return invoke_or_recover(
          [&]() -> keyed_slot_map_result<void> {
             self._data.construct_at(index, std::forward<Args>(args)...);
             return {};
          },
          [&]() -> keyed_slot_map_result<void> {
             // construct_at()は例外送出時にコンテナの状態を変更しない。
             self._domain.erase(ret_value.first);
             self._data.cancel(index);
             return make_unexpected(errc_type::failed_to_constructed);
          });
   }

   /**
    * @brief erase key with routed
    * @note
    * 削除された場合、falseを返す。
    * 例外が送出された場合、指定した要素は削除されない。
    */
   template <class Self, class LocKeyT>
   auto routed_erase(this Self &self, LocKeyT const &key) -> bool {
      auto const ite = self._domain.find(key);
      if (ite == self._domain.end()) [[unlikely]] {
         return false;
      }
      // keyが存在しない、またはデストラクタが例外を送出する可能性がある。
      // ただし、例外が送出された場合、要素の削除は行われない。
      self._data.erase(ite->second);
      // unordered_mapのerase()はnoexceptではないが、
      // iteratorを利用した場合は例外を投げない。
      self._domain.erase(ite);
      return true;
   }

   template <class LocKeyT>
   [[nodiscard]] auto require_routed_exist(this self_type const &self,
                                           LocKeyT const &key)
       -> keyed_slot_map_result<void> {
      auto const ite = self._domain.find(key);
      if (ite == self._domain.end()) [[unlikely]] {
         return make_unexpected(errc_type::key_was_not_contain);
      }
      if (!self._data.contains(ite->second)) [[unlikely]] {
         return make_unexpected(errc_type::failed_to_domain_not_be_resolved);
      }
      return {};
   }

   /**
    * @brief keyのみの存在確認。
    * @note
    * keyが存在しない場合のみの処理を行いたい場合に利用することを想定しています。
    */
   template <class LocKeyT>
   [[nodiscard]] auto contains(this self_type const &self, LocKeyT const &key)
       -> bool {
      return self._domain.contains(key);
   }

   auto operator=(keyed_slot_map const &rhs) -> keyed_slot_map & = delete;
   auto operator=(keyed_slot_map &&rhs) -> keyed_slot_map & = delete;
};

} // namespace maboroutu
