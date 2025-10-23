#pragma once

#include <concepts>
#include <initializer_list>
#include <memory>
#include <utility>
namespace maboroutu {
namespace ios {

template <class T>
concept StreamBufConfig = requires(T &Value) {
  typename T::char_type;
  typename T::traits_type;
  {
    Value.rdbuf()->pubsetbuf(nullptr, 0)
  } -> std::same_as<
      std::basic_streambuf<typename T::char_type, typename T::traits_type> *>;
};

template <StreamBufConfig StreamBufConfigT, size_t BufSizeV = 4 * (1024 * 1024)>
class IOStreamBuffer {
public: /*STRUCT_FIELD*/
  using value_type = StreamBufConfigT;
  using char_type = typename value_type::char_type;
  using traits_type = typename value_type::traits_type;

protected:
private:
  static constexpr size_t ExtBufSize = 0x10;
  static constexpr size_t DefaultBufferSize = BufSizeV;
  static constexpr size_t DefaultAllocateSize = BufSizeV + ExtBufSize;

  std::unique_ptr<char_type[]> Buffer;
  size_t BufferSize;
  value_type Value;

private: /*IMPLIMENT_FIELD*/
protected:
  constexpr inline void reallocatebuf(size_t AllocateBufferSize) {
    if (BufferSize == AllocateBufferSize) [[unlikely]] {
      return;
    }
    BufferSize = AllocateBufferSize;
    Buffer = std::make_unique<char_type[]>(BufferSize + ExtBufSize);
  }

public:
  IOStreamBuffer()
      : Buffer(std::make_unique<char_type[]>(DefaultAllocateSize)),
        BufferSize(DefaultBufferSize), Value() {}
  IOStreamBuffer(IOStreamBuffer const &Rhs) = delete;
  IOStreamBuffer(IOStreamBuffer &&Rhs) = default;
  IOStreamBuffer(value_type &&RhsValue)
      : Buffer(std::make_unique<char_type[]>(DefaultAllocateSize)),
        BufferSize(DefaultBufferSize), Value(std::move(RhsValue)) {
    setbuf();
  }
  IOStreamBuffer(value_type &&RhsValue, size_t const BufferSize)
      : Buffer(std::make_unique<char_type[]>(BufferSize + ExtBufSize)),
        BufferSize(BufferSize), Value(std::move(RhsValue)) {
    setbuf();
  }
  ~IOStreamBuffer() = default;

  IOStreamBuffer &operator=(IOStreamBuffer const &) = delete;
  IOStreamBuffer &operator=(IOStreamBuffer &&Rhs) {
    Buffer = std::move(Rhs.Buffer);
    BufferSize = Rhs.BufferSize;
    Value = std::move(Rhs.Value);

    return *this;
  }
  IOStreamBuffer &operator=(value_type &&RhsValue) {
    Value = std::move(RhsValue);
    setbuf();

    return *this;
  }

  constexpr value_type &operator*() noexcept { return Value; }
  constexpr value_type const &operator*() const noexcept { return Value; }

  constexpr value_type *operator->() noexcept { return &Value; }
  constexpr value_type const *operator->() const noexcept { return &Value; }

  constexpr value_type &value() noexcept { return Value; }
  constexpr value_type const &value() const noexcept { return Value; }

  template <class... ArgsT> constexpr value_type emplace(ArgsT... Args) {

    Value(std::forward<ArgsT>(Args)...);
    setbuf();

    return Value;
  }
  template <class LocT, class... ArgsT>
  constexpr value_type &emplace(std::initializer_list<LocT> InitialList,
                                ArgsT... Args) {
    Value(InitialList, std::forward<ArgsT>(Args)...);
    setbuf();

    return Value;
  }

  void setbuf() {
    if (auto *const StreamBuf = Value.rdbuf()) [[likely]] {
      StreamBuf->pubsetbuf(Buffer.get(), BufferSize);
    }
  }
  void setbuf(size_t BufferSize) {
    reallocatebuf(BufferSize);
    setbuf();
  }
};
} // namespace ios
} // namespace maboroutu
