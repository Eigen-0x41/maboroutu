module;
#include <concepts>
#include <mutex>
#include <shared_mutex>
#include <type_traits>
#include <utility>
export module maboroutu.mutex_wrapper;
import maboroutu.core;

namespace maboroutu {

// [[null_mutex]]
export class null_mutex {
 public: /*STRUCT_FIELD*/
 protected:
 private:
   using self_type = null_mutex;
   /*--:  *IMPLIMENT_FIELD*/
 protected:
 public:
   null_mutex() noexcept = default;
   null_mutex(null_mutex const &) = delete;
   null_mutex(null_mutex &&) = default;
   ~null_mutex() = default;

   static constexpr void lock() noexcept {}
   static constexpr bool try_lock() noexcept { return true; }
   static constexpr void unlock() noexcept {}

   auto operator=(null_mutex const &other) -> null_mutex & = delete;
   auto operator=(null_mutex &&other) -> null_mutex & = default;
};
export using std::mutex;
export using std::shared_mutex;

template <class T> struct mutex_traits : public std::false_type {
   static_assert(false, "using default template.");
};

template <> struct mutex_traits<null_mutex> : public std::true_type {
   using mutex_type = null_mutex;
   using read_lock_type = std::scoped_lock<mutex_type>;
   using write_lock_type = std::scoped_lock<mutex_type>;
};
template <> struct mutex_traits<mutex> : public std::true_type {
   using mutex_type = mutex;
   using read_lock_type = std::scoped_lock<mutex_type>;
   using write_lock_type = std::scoped_lock<mutex_type>;
};
template <> struct mutex_traits<shared_mutex> : public std::true_type {
   using mutex_type = shared_mutex;
   using read_lock_type = std::shared_lock<mutex_type>;
   using write_lock_type = std::scoped_lock<mutex_type>;
};

// [[mutex_wrapper]]
export template <class T, class Mutex, class Traits = mutex_traits<Mutex>>
class mutex_wrapper {
 public: /*STRUCT_FIELD*/
   using value_type = T;
   using mutex_type = Mutex;
   using traits_type = Traits;

 protected:
 private:
   using self_type = mutex_wrapper;

   value_type _value{};
   [[no_unique_address]] mutable mutex_type _mutex;
   /*--:  *IMPLIMENT_FIELD*/
 protected:
 public:
   mutex_wrapper() = default;
   mutex_wrapper(mutex_wrapper const &) = default;
   mutex_wrapper(mutex_wrapper &&) = default;
   template <class... Args>
      requires(!one_of<mutex_wrapper, std::remove_cvref_t<Args>...>)
   mutex_wrapper(Args &&...args) : _value(std::forward<Args>(args)...) {}
   ~mutex_wrapper() = default;

   template <class Self, class F>
   constexpr auto read_lock(this Self &self, F &&func) noexcept(
       std::is_nothrow_invocable_v<F &&, value_type const &>)
       -> std::invoke_result_t<F &&, value_type const &> {
      auto lock = typename traits_type::read_lock_type(self._mutex);
      return std::invoke(std::forward<F>(func), std::as_const(self._value));
   }

   template <class Self, class F>
      requires(!std::is_const_v<Self>)
   constexpr auto write_lock(this Self &self, F &&func) noexcept(
       std::is_nothrow_invocable_v<F &&, copy_const_t<Self, value_type &>>)
       -> std::invoke_result_t<F &&, copy_const_t<Self, value_type &>> {
      auto lock = typename traits_type::write_lock_type(self._mutex);
      return std::invoke(std::forward<F>(func), self._value);
   }

   auto operator=(mutex_wrapper const &other) -> mutex_wrapper & = default;
   auto operator=(mutex_wrapper &&other) -> mutex_wrapper & = default;
};


} // namespace maboroutu
