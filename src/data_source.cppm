// module;
// #include <cstddef>
// #include <expected>
// #include <vector>
export module maboroutu.data_source;
// import maboroutu.core;
// import maboroutu.error;
//
// namespace maboroutu {
//
// export template <class T>
// concept data_source = requires(T &src, region r) {
//    { src.read(r) } -> std::same_as<std::expected<byte_array, error>>;
//    { src.size() } -> std::same_as<std::expected<std::size_t, error>>;
// };
//
// export template <class T>
// concept data_sink = requires(T &sink, region r, const byte_array &data) {
//    { sink.write(r, data) } -> std::same_as<std::expected<void, error>>;
// };
//
// export class file_data_source {
//  public:
//    auto read(region r) -> std::expected<byte_array, error>;
//    auto write(region r, const byte_array &data) -> std::expected<void, error>;
//    auto size() const -> std::expected<std::size_t, error>;
// };
// static_assert(data_source<file_data_source>);
// static_assert(data_sink<file_data_source>);
//
// export class data_source_handle {
//    struct _concept {
//       virtual ~_concept() = default;
//       virtual auto read(region r) -> std::expected<byte_array, error> = 0;
//       virtual auto size() const -> std::expected<std::size_t, error> = 0;
//    };
//
//    template <data_source T> struct _model final : _concept {
//       T _value;
//       explicit _model(T v) : _value(std::move(v)) {}
//       auto read(region r) -> std::expected<byte_array, error> override {
//          return _value.read(r);
//       }
//       auto size() const -> std::expected<std::size_t, error> override {
//          return _value.size();
//       }
//    };
//
//    std::unique_ptr<_concept> _value;
//
//  public:
//    template <data_source T>
//    data_source_handle(T v)
//        : _value(std::make_unique<_model<T>>(std::move(v))) {}
//
//    auto read(region r) { return _value->read(r); }
//    auto size() const { return _value->size(); }
// };
// } // namespace maboroutu
