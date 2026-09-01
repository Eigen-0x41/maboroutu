module;
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <span>
export module maboroutu.data_source;
export import maboroutu.core;
import maboroutu.error;

namespace maboroutu {

export namespace errc {
enum class data_source {
   out_of_range,
   invalid_member_variable,
   operation_failure,
};
} // namespace errc

export template <class T>
using data_source_result = result<T, errc::data_source>;

export template <class T>
concept data_source = requires(T &src, region r) {
   typename T::template result_type<void>;
   requires std::same_as<typename T::template result_type<void>,
                         data_source_result<void>>;

   { src.read(r) } -> std::same_as<data_source_result<byte_array>>;
   { src.size() } -> std::same_as<data_source_result<std::size_t>>;
};

export template <class T>
concept writable_data_source =
    data_source<T> &&
    requires(T &src, region r, std::span<std::byte const> data) {
       { src.write(r, data) } -> std::same_as<data_source_result<void>>;
    };

export class data_source_handle {
   struct _concept {
      virtual ~_concept() = default;
      [[nodiscard]] virtual auto read(region r)
          -> data_source_result<byte_array> = 0;
      [[nodiscard]] virtual auto size() const
          -> data_source_result<std::size_t> = 0;
      [[nodiscard]] virtual auto try_clone() const
          -> std::unique_ptr<_concept> = 0;
   };

   template <data_source T> struct _model final : _concept {
      T value;
      explicit _model(T v) : value(std::move(v)) {}
      [[nodiscard]] auto read(region r)
          -> data_source_result<byte_array> override {
         return value.read(r);
      }
      [[nodiscard]] auto size() const
          -> data_source_result<std::size_t> override {
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

   explicit data_source_handle(std::unique_ptr<_concept> value)
       : _value(std::move(value)) {}

 public:
   template <data_source T>
   data_source_handle(T value)
       : _value(std::make_unique<_model<T>>(std::move(value))) {}

   [[nodiscard]] auto read(region r) { return _value->read(r); }
   [[nodiscard]] auto size() const { return _value->size(); }

   [[nodiscard]] auto try_copy(this data_source_handle const &self)
       -> std::optional<data_source_handle> {
      if (auto cloned = self._value->try_clone()) {
         return data_source_handle(std::move(cloned));
      }
      return std::nullopt;
   }
};

export class writable_data_source_handle {
   struct _concept {
      virtual ~_concept() = default;
      [[nodiscard]] virtual auto read(region r)
          -> data_source_result<byte_array> = 0;
      [[nodiscard]] virtual auto write(region r,
                                       std::span<std::byte const> data)
          -> data_source_result<void> = 0;
      [[nodiscard]] virtual auto size() const
          -> data_source_result<std::size_t> = 0;
      [[nodiscard]] virtual auto try_clone() const
          -> std::unique_ptr<_concept> = 0;
   };

   template <writable_data_source T> struct _model final : _concept {
      T value;
      explicit _model(T v) : value(std::move(v)) {}
      [[nodiscard]] auto read(region r)
          -> data_source_result<byte_array> override {
         return value.read(r);
      }
      [[nodiscard]] auto write(region r, std::span<std::byte const> data)
          -> data_source_result<void> override {
         return value.write(r, data);
      }
      [[nodiscard]] auto size() const
          -> data_source_result<std::size_t> override {
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

   explicit writable_data_source_handle(std::unique_ptr<_concept> value)
       : _value(std::move(value)) {}

 public:
   template <writable_data_source T>
   writable_data_source_handle(T v)
       : _value(std::make_unique<_model<T>>(std::move(v))) {}

   [[nodiscard]] auto read(region r) { return _value->read(r); }
   [[nodiscard]] auto write(region r, std::span<std::byte const> data) {
      return _value->write(r, data);
   }
   [[nodiscard]] auto size() const { return _value->size(); }

   [[nodiscard]] auto try_copy(this writable_data_source_handle const &self)
       -> std::optional<writable_data_source_handle> {
      if (auto cloned = self._value->try_clone()) {
         return writable_data_source_handle(std::move(cloned));
      }
      return std::nullopt;
   }
};

} // namespace maboroutu
