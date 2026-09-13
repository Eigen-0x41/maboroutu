module;
#include <concepts>
#include <memory>
#include <span>
export module maboroutu.data_buffer;
export import maboroutu.core;
export import maboroutu.error;

import maboroutu.data_source;

namespace maboroutu {

namespace errc {
enum class data_buffer {
   out_of_range,
   operation_failure,
};
}

export template <class T>
using data_buffer_result = result<T, errc::data_buffer>;

export template <class T>
concept data_buffer =
    writable_data_source<T> &&
    requires(T &buf, std::size_t n, region r, std::span<std::byte const> data) {
       { buf.append(data) } -> std::same_as<data_buffer_result<region>>;
       { buf.grow(n) } -> std::same_as<data_buffer_result<region>>;
       {
          buf.view(r)
       } -> std::same_as<data_buffer_result<std::span<std::byte>>>;
    };

// NOTE: concept検証兼 posix /dev/null の模倣
export struct null_data_buffer {
   template <class T> using result_type = data_source_result<T>;

   [[nodiscard]] static auto size() -> result_type<std::size_t> { return 0; }
   [[nodiscard]] static auto read(region r) -> result_type<byte_array> {
      return byte_array{
          .value = std::make_unique<byte_array::value_type>(r.size),
          .size = r.size,
      };
   }
   [[nodiscard]] auto write(region, std::span<std::byte const>)
       -> result_type<void> {
      return {};
   }

   static auto append(std::span<std::byte const> n)
       -> data_buffer_result<region> {
      return region{
          .offset = 0,
          .size = 0,
      };
   }
   static auto grow(std::size_t n) -> data_buffer_result<region> {
      return region{
          .offset = 0,
          .size = 0,
      };
   }
   static auto view(region r) -> data_buffer_result<std::span<std::byte>> {
      return make_unexpected(
          data_buffer_result<void>::error_type::code_type::out_of_range);
   }
};
static_assert(data_buffer<null_data_buffer>, "");

} // namespace maboroutu
