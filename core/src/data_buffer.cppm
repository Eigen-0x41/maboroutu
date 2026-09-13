module;
#include <concepts>
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

} // namespace maboroutu
