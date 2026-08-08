module;
#include <cstddef>
#include <expected>
#include <vector>
export module maboroutu.data_source;
import maboroutu.core;
import mylib.error;

namespace maboroutu {

export template <class T>
concept data_source = requires(T &src, mylib::region r) {
   {
      src.read(r)
   } -> std::same_as<std::expected<mylib::byte_array, mylib::error>>;
   { src.size() } -> std::same_as<std::expected<std::size_t, mylib::error>>;
};

export template <class T>
concept data_sink =
    requires(T &sink, mylib::region r, const mylib::byte_array &data) {
       {
          sink.write(r, data)
       } -> std::same_as<std::expected<void, mylib::error>>;
    };

export class file_data_source {
 public:
   auto read(mylib::region r) -> std::expected<mylib::byte_array, mylib::error>;
   auto write(mylib::region r, const mylib::byte_array &data)
       -> std::expected<void, mylib::error>;
   auto size() const -> std::expected<std::size_t, mylib::error>;
};
static_assert(data_source<file_data_source>);
static_assert(data_sink<file_data_source>);

export class data_source_handle {
   struct _concept {
      virtual ~_concept() = default;
      virtual auto read(mylib::region r)
          -> std::expected<mylib::byte_array, mylib::error> = 0;
      virtual auto size() const -> std::expected<std::size_t, mylib::error> = 0;
   };

   template <data_source T> struct _model final : _concept {
      T _value;
      explicit _model(T v) : _value(std::move(v)) {}
      auto read(mylib::region r)
          -> std::expected<mylib::byte_array, mylib::error> override {
         return _value.read(r);
      }
      auto size() const -> std::expected<std::size_t, mylib::error> override {
         return _value.size();
      }
   };

   std::unique_ptr<_concept> _value;

 public:
   template <data_source T>
   data_source_handle(T v)
       : _value(std::make_unique<_model<T>>(std::move(v))) {}

   auto read(mylib::region r) { return _value->read(r); }
   auto size() const { return _value->size(); }
};
} // namespace maboroutu
