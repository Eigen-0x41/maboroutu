module;
#include <concepts>
export module maboroutu.io_pattern;
export import maboroutu.error;

namespace maboroutu {

// TODO: エラー値の選定
export namespace errc {
enum class io_pattern {};
} // namespace errc

export template <class T> using io_pattern_result = result<T, errc::io_pattern>;

export template <typename Raw, typename Source, class... Args>
concept reader = requires(Source &src) {
   { read_from(src) } -> std::same_as<io_pattern_result<Raw>>;
};

export template <typename Raw, typename Destination, class... Args>
concept writer = requires(const Raw &raw, Destination &dst) {
   { write_to(raw, dst) } -> std::same_as<io_pattern_result<void>>;
};

export template <typename Raw, typename Domain, class... Args>
concept mapper = requires(const Raw &raw) {
   { to_domain(raw) } -> std::same_as<io_pattern_result<Domain>>;
};
// from_domainはDomain自身が実装する。

} // namespace maboroutu
