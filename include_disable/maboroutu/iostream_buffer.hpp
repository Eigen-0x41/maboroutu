#pragma once

#include <concepts>
#include <initializer_list>
#include <memory>
#include <utility>

namespace maboroutu {

// static constexpr size_t kib_size = 1024;
static constexpr size_t kib_size = 0x01 << 10; // pow(2, 10);
static constexpr size_t mib_size = 0x01 << 20; // pow(2, 20);

template <class T>
concept stream_buf_config = requires(T &value) {
  typename T::char_type;
  typename T::traits_type;
  {
    value.rdbuf()->pubsetbuf(nullptr, 0)
  } -> std::same_as<
      std::basic_streambuf<typename T::char_type, typename T::traits_type> *>;
};

template <stream_buf_config StreamBufConfigT, size_t BufSizeV = 4 * mib_size>
class iostream_buffer {
public: /*STRUCT_FIELD*/
  using value_type = StreamBufConfigT;
  using char_type = typename value_type::char_type;
  using traits_type = typename value_type::traits_type;

protected:
private:
  static constexpr size_t ext_buf_size = 0x10;
  static constexpr size_t default_buffer_size = BufSizeV;
  static constexpr size_t default_allocate_size = BufSizeV + ext_buf_size;

  // std::vector is allocating more than requested size.
  // but buffer size is best in requested.
  using buffer_mapped_type = char_type[];

  std::unique_ptr<buffer_mapped_type> _buffer;
  size_t _buffer_size;
  value_type _stream;

private: /*IMPLIMENT_FIELD*/
protected:
  constexpr void reallocatebuf(size_t allocate_buffer_size) {
    if (_buffer_size == allocate_buffer_size) [[unlikely]] {
      return;
    }
    _buffer_size = allocate_buffer_size;
    _buffer = std::make_unique<buffer_mapped_type>(_buffer_size + ext_buf_size);
  }

public:
  iostream_buffer()
      : _buffer(std::make_unique<buffer_mapped_type>(default_allocate_size)),
        _buffer_size(default_buffer_size), _stream() {}
  iostream_buffer(iostream_buffer const &rhs) = delete;
  iostream_buffer(iostream_buffer &&rhs) = default;
  iostream_buffer(value_type &&rhs_value)
      : _buffer(std::make_unique<buffer_mapped_type>(default_allocate_size)),
        _buffer_size(default_buffer_size), _stream(std::move(rhs_value)) {
    setbuf();
  }
  iostream_buffer(value_type &&rhs_value, size_t const buffer_size)
      : _buffer(
            std::make_unique<buffer_mapped_type>(buffer_size + ext_buf_size)),
        _buffer_size(buffer_size), _stream(std::move(rhs_value)) {
    setbuf();
  }
  ~iostream_buffer() = default;

  constexpr auto operator=(iostream_buffer const &)
      -> iostream_buffer & = delete;
  constexpr auto operator=(iostream_buffer &&rhs) noexcept
      -> iostream_buffer & {
    _buffer = std::move(rhs._buffer);
    _buffer_size = rhs._buffer_size;
    _stream = std::move(rhs._stream);

    return *this;
  }
  constexpr auto operator=(value_type &&rhs_value) -> iostream_buffer & {
    _stream = std::move(rhs_value);
    setbuf();

    return *this;
  }

  constexpr auto operator*() noexcept -> value_type & { return _stream; }
  constexpr auto operator*() const noexcept -> value_type const & {
    return _stream;
  }

  constexpr auto operator->() noexcept -> value_type * { return &_stream; }
  constexpr auto operator->() const noexcept -> value_type const * {
    return &_stream;
  }

  constexpr auto value() & noexcept -> value_type & { return _stream; }
  constexpr auto value() const & noexcept -> value_type const & {
    return _stream;
  }
  constexpr auto value() && noexcept -> value_type && { return _stream; }
  constexpr auto value() const && noexcept -> value_type const && {
    return _stream;
  }

  template <class... ArgsT>
  constexpr auto emplace(ArgsT... args) -> value_type {

    _stream(std::forward<ArgsT>(args)...);
    setbuf();

    return _stream;
  }
  template <class LocT, class... ArgsT>
  constexpr auto emplace(std::initializer_list<LocT> initial_list,
                         ArgsT... args) -> value_type & {
    _stream(initial_list, std::forward<ArgsT>(args)...);
    setbuf();

    return _stream;
  }

  constexpr void setbuf() {
    if (auto *const stream_buf = _stream.rdbuf()) [[likely]] {
      stream_buf->pubsetbuf(_buffer.get(), _buffer_size);
    }
  }
  constexpr void setbuf(size_t buffer_size) {
    reallocatebuf(buffer_size);
    setbuf();
  }
};
} // namespace maboroutu
