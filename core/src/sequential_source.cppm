module;
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <span>
export module maboroutu.sequential_source;
export import maboroutu.core;
export import maboroutu.error;

namespace maboroutu {

namespace errc {
enum class sequential_source {
   out_of_range,
   invalid_member_variable,
   operation_failure,
};
} // namespace errc

export template <class T>
using sequential_source_result = result<T, errc::sequential_source>;

export template <class T>
concept sequential_source = requires(T &src, std::size_t n) {
   typename T::template result_type<void>;
   requires std::same_as<typename T::template result_type<void>,
                         sequential_source_result<void>>;

   { src.count() } -> std::same_as<std::size_t>;
   { src.read(n) } -> std::same_as<sequential_source_result<byte_array>>;
};

// write()はread()と同一のcount()を共有する（読み書きどちらが成功しても
// 同一の前進済みバイト数として扱う、一時的な位置情報であるため）。
export template <class T>
concept writable_sequential_source =
    sequential_source<T> && requires(T &dst, std::span<std::byte const> data) {
       { dst.write(data) } -> std::same_as<sequential_source_result<void>>;
    };

class writable_sequential_source_handle {
   struct _concept {
      virtual ~_concept() = default;
      [[nodiscard]] virtual auto read(std::size_t)
          -> sequential_source_result<byte_array> = 0;
      [[nodiscard]] virtual auto write(std::span<std::byte const>)
          -> sequential_source_result<void> = 0;
      [[nodiscard]] virtual auto count() const -> std::size_t = 0;
      [[nodiscard]] virtual auto try_clone() const
          -> std::unique_ptr<_concept> = 0;
   };

   template <writable_sequential_source T> struct _model final : _concept {
      T value;
      explicit _model(T v) : value(std::move(v)) {}
      [[nodiscard]] auto read(std::size_t size)
          -> sequential_source_result<byte_array> override {
         return value.read(size);
      }
      [[nodiscard]] auto write(std::span<std::byte const> data)
          -> sequential_source_result<void> override {
         return value.write(data);
      }
      [[nodiscard]] auto count() const -> std::size_t override {
         return value.size();
      }
      [[nodiscard]] auto try_clone() const
          -> std::unique_ptr<_concept> override {
         if constexpr (std::copy_constructible<T>) {
            return std::make_unique<_model<T>>(value);
         } else {
            return nullptr;
         }
      }
   };

   std::unique_ptr<_concept> _value;

   explicit writable_sequential_source_handle(std::unique_ptr<_concept> value)
       : _value(std::move(value)) {}

 public:
   template <class T> using result_type = sequential_source_result<T>;

   template <class T>
   writable_sequential_source_handle(T value)
      requires writable_sequential_source<T> &&
               (!std::same_as<std::remove_cvref_t<T>,
                              writable_sequential_source_handle>)
       : _value(std::make_unique<_model<T>>(std::move(value))) {}

   [[nodiscard]] auto read(std::size_t size) { return _value->read(size); }
   [[nodiscard]] auto write(std::span<std::byte const> data) {
      return _value->write(data);
   }
   [[nodiscard]] auto count() const { return _value->count(); }

   [[nodiscard]] auto
   try_copy(this writable_sequential_source_handle const &self)
       -> std::optional<writable_sequential_source_handle> {
      if (auto cloned = self._value->try_clone()) {
         return writable_sequential_source_handle(std::move(cloned));
      }
      return std::nullopt;
   }
};

static_assert(sequential_source<writable_sequential_source_handle>, "");
static_assert(writable_sequential_source<writable_sequential_source_handle>,
              "");
static_assert(!std::is_constructible_v<writable_sequential_source_handle>, "");
static_assert(!std::is_copy_constructible_v<writable_sequential_source_handle>,
              "");
static_assert(std::is_move_constructible_v<writable_sequential_source_handle>,
              "");
static_assert(!std::is_constructible_v<writable_sequential_source_handle,
                                       writable_sequential_source_handle &>,
              "");

class sequential_source_handle {
   struct _concept {
      virtual ~_concept() = default;
      [[nodiscard]] virtual auto read(std::size_t)
          -> sequential_source_result<byte_array> = 0;
      [[nodiscard]] virtual auto count() const -> std::size_t = 0;
      [[nodiscard]] virtual auto try_clone() const
          -> std::unique_ptr<_concept> = 0;
   };

   template <sequential_source T> struct _model final : _concept {
      T value;
      explicit _model(T v) : value(std::move(v)) {}
      [[nodiscard]] auto read(std::size_t size)
          -> sequential_source_result<byte_array> override {
         return value.read(size);
      }
      [[nodiscard]] auto count() const -> std::size_t override {
         return value.size();
      }
      [[nodiscard]] auto try_clone() const
          -> std::unique_ptr<_concept> override {
         if constexpr (std::copy_constructible<T>) {
            return std::make_unique<_model<T>>(value);
         } else {
            return nullptr;
         }
      }
   };

   std::unique_ptr<_concept> _value;

   explicit sequential_source_handle(std::unique_ptr<_concept> value)
       : _value(std::move(value)) {}

 public:
   template <class T> using result_type = sequential_source_result<T>;

   sequential_source_handle() = delete;
   sequential_source_handle(sequential_source_handle const &) = delete;
   sequential_source_handle(sequential_source_handle &&) = default;
   template <class T>
   sequential_source_handle(T value)
      requires sequential_source<T> &&
               (!std::same_as<std::remove_cvref_t<T>,
                              sequential_source_handle>) &&
               (!std::same_as<std::remove_cvref_t<T>,
                              writable_sequential_source_handle>)
       : _value(std::make_unique<_model<T>>(std::move(value))) {}

   [[nodiscard]] auto read(std::size_t size) { return _value->read(size); }
   [[nodiscard]] auto count() const { return _value->count(); }

   [[nodiscard]] auto try_copy(this sequential_source_handle const &self)
       -> std::optional<sequential_source_handle> {
      if (auto cloned = self._value->try_clone()) {
         return sequential_source_handle(std::move(cloned));
      }
      return std::nullopt;
   }
};
static_assert(sequential_source<sequential_source_handle>, "");
static_assert(!writable_sequential_source<sequential_source_handle>, "");
static_assert(!std::is_constructible_v<sequential_source_handle>, "");
static_assert(!std::is_copy_constructible_v<sequential_source_handle>, "");
static_assert(std::is_move_constructible_v<sequential_source_handle>, "");
static_assert(!std::is_constructible_v<sequential_source_handle,
                                       sequential_source_handle &>,
              "");

// NOTE: concept検証兼 posix /dev/null の模倣
export struct null_sequential_source {
   template <class T> using result_type = sequential_source_result<T>;

   [[nodiscard]] static auto count() -> std::size_t { return 0; }
   [[nodiscard]] static auto read(std::size_t size)
       -> sequential_source_result<byte_array> {
      return byte_array{
          .value = std::make_unique<byte_array::value_type>(size),
          .size = size,
      };
   }
};
static_assert(sequential_source<null_sequential_source>, "");
static_assert(!writable_sequential_source<null_sequential_source>, "");
static_assert(
    std::is_constructible_v<sequential_source_handle, null_sequential_source &>,
    "");
static_assert(!std::is_constructible_v<writable_sequential_source_handle,
                                       null_sequential_source &>,
              "");

export struct null_writable_sequential_source {
   template <class T> using result_type = sequential_source_result<T>;

   [[nodiscard]] static auto count() noexcept -> std::size_t { return 0; }
   [[nodiscard]] static auto read(std::size_t size) noexcept
       -> sequential_source_result<byte_array> {
      return byte_array{
          .value = std::make_unique<byte_array::value_type>(size),
          .size = size,
      };
   }
   [[nodiscard]] static auto
   write(std::span<std::byte const> /*unused*/) noexcept
       -> sequential_source_result<void> {
      return {};
   }
};
static_assert(sequential_source<null_writable_sequential_source>, "");
static_assert(writable_sequential_source<null_writable_sequential_source>, "");
static_assert(std::is_constructible_v<sequential_source_handle,
                                      null_writable_sequential_source &>,
              "");
static_assert(std::is_constructible_v<writable_sequential_source_handle,
                                      null_writable_sequential_source &>,
              "");

} // namespace maboroutu
