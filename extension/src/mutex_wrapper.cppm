module;
#include <concepts>
#include <mutex>
#include <shared_mutex>
#include <type_traits>
export module maboroutu.mutex_wrapper;

namespace maboroutu {

template <class T> struct mutex_traits : public std::false_type {
   static_assert(false, "using default template.");
};

struct dummy_mutex {};

template <> struct mutex_traits<void> : public std::true_type {
   using mutex_type = dummy_mutex;
   using read_lock_type = void;
   using write_lock_type = void;
};
template <> struct mutex_traits<std::mutex> : public std::true_type {
   using mutex_type = std::mutex;
   using read_lock_type = std::scoped_lock<std::mutex>;
   using write_lock_type = std::scoped_lock<std::mutex>;
};
template <> struct mutex_traits<std::shared_mutex> : public std::true_type {
   using mutex_type = std::shared_mutex;
   using read_lock_type = std::shared_lock<std::shared_mutex>;
   using write_lock_type = std::scoped_lock<std::shared_mutex>;
};

template <class Lock> constexpr auto get_lock(auto &mutex) -> decltype(auto) {
   return Lock(mutex);
}
template <>
constexpr auto get_lock<void>(dummy_mutex &mutex) -> decltype(auto) {
   return mutex;
}

template <class F, class T, class MutexTraits>
struct const_arg_when_read
    : public std::conditional<
          !std::is_invocable_v<F, std::remove_cvref_t<T> const &>,
          typename MutexTraits::read_lock_type,
          typename MutexTraits::write_lock_type> {};

template <class F, class T, class MutexTraits>
using const_arg_when_read_t = const_arg_when_read<F, T, MutexTraits>::type;

// [[mutex_wrapper]]
export template <class T, class Mutex = std::mutex,
                 class Traits = mutex_traits<Mutex>>
class mutex_wrapper {
 public: /*STRUCT_FIELD*/
   using value_type = T;
   using mutex_type = Mutex;
   using traits_type = Traits;

 protected:
 private:
   using self_type = mutex_wrapper;

   value_type _value;
   [[no_unique_address]] mutable mutex_type _mutex;
   /*--:  *IMPLIMENT_FIELD*/
 protected:
 public:
   mutex_wrapper() = default;
   mutex_wrapper(mutex_wrapper const &) = default;
   mutex_wrapper(mutex_wrapper &&) = default;
   ~mutex_wrapper() = default;

   template <class F, class Lock = const_arg_when_read_t<F, T, traits_type>>
   constexpr auto lock(F &&func) noexcept(noexcept(func(_mutex)))
       -> decltype(auto) {
      auto lock = get_lock<Lock>(_mutex);
      return func(_value);
   }

   auto operator=(mutex_wrapper const &other) -> mutex_wrapper & = default;
   auto operator=(mutex_wrapper &&other) -> mutex_wrapper & = default;
};

namespace {
using test_mutex_wrapper = mutex_wrapper<int>;

static_assert(
    std::same_as<
        const_arg_when_read_t<decltype([](test_mutex_wrapper::value_type &) {}),
                              test_mutex_wrapper::value_type,
                              test_mutex_wrapper::traits_type>,
        test_mutex_wrapper::traits_type::write_lock_type>,
    "");
static_assert(
    std::same_as<
        const_arg_when_read_t<
            decltype([](test_mutex_wrapper::value_type const &) {}),
            test_mutex_wrapper::value_type, test_mutex_wrapper::traits_type>,
        test_mutex_wrapper::traits_type::read_lock_type>,
    "");

using test_shared_mutex_wrapper = mutex_wrapper<int>;

static_assert(
    std::same_as<const_arg_when_read_t<
                     decltype([](test_shared_mutex_wrapper::value_type &) {}),
                     test_shared_mutex_wrapper::value_type,
                     test_shared_mutex_wrapper::traits_type>,
                 test_shared_mutex_wrapper::traits_type::write_lock_type>,
    "");
static_assert(
    std::same_as<
        const_arg_when_read_t<
            decltype([](test_shared_mutex_wrapper::value_type const &) {}),
            test_shared_mutex_wrapper::value_type,
            test_shared_mutex_wrapper::traits_type>,
        test_shared_mutex_wrapper::traits_type::read_lock_type>,
    "");
} // namespace

} // namespace maboroutu
