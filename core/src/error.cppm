module;
#include <concepts>
#include <cstddef>
#include <expected>
#include <memory>
#include <type_traits>
#include <utility>
#include <variant>
#include <version>
export module maboroutu.error;
export import maboroutu.core;

#if __STDC_HOSTED__ == 0
#if !defined(__cpp_lib_freestanding_expected)
static_assert(
    false,
    "maboroutu requires a freestanding implementation providing <expected> "
    "(__cpp_lib_freestanding_expected is not defined). "
    "A C++26 or later freestanding-conforming standard library is required.");
#endif
#if !defined(__cpp_lib_freestanding_variant)
static_assert(
    false,
    "maboroutu requires a freestanding implementation providing <variant> "
    "(__cpp_lib_freestanding_variant is not defined).");
#endif
#if !defined(__cpp_lib_freestanding_memory)
static_assert(false,
              "maboroutu requires a freestanding implementation providing "
              "<memory>/unique_ptr "
              "(__cpp_lib_freestanding_memory is not defined).");
#endif
#endif

namespace maboroutu {

// [[detail_type]]
template <class T, class D>
   requires std::is_object_v<T>
class error_detail {
 public: /*STRUCT_FIELD*/
   using detail_type = T;

 protected:
 private:
   using self_type = error_detail;
   using value_type = std::unique_ptr<detail_type, D>;

   // unique_ptrについては仕様の通りでないと例外要件が破綻するため
   // 過剰ではあるがテストを行う。
   // default
   static_assert(std::is_nothrow_constructible_v<value_type, std::nullptr_t>,
                 "<std::unique_ptr<detail_type, D>> can nothrow construcable "
                 "with nullptr_t");
   // copy
   static_assert(std::is_nothrow_assignable_v<value_type, std::nullptr_t>,
                 "<std::unique_ptr<detail_type, D>> can nothrow assignable");
   // move
   static_assert(
       std::is_nothrow_move_constructible_v<value_type>,
       "<std::unique_ptr<detail_type, D>> can nothrow move construcable");
   static_assert(
       std::is_nothrow_move_assignable_v<value_type>,
       "<std::unique_ptr<detail_type, D>> can nothrow move assignable");
   // destruct
   static_assert(std::is_nothrow_destructible_v<value_type>,
                 "<std::unique_ptr<detail_type, D>> can nothrow destructible");

   // deleter destruct
   static_assert(std::is_nothrow_destructible_v<D>,
                 "<class D> is can nothrow destructible");
   // deleter move
   static_assert(std::is_nothrow_move_constructible_v<D>,
                 "<class D> is can nothrow move assignable");
   static_assert(std::is_nothrow_move_assignable_v<D>,
                 "<class D> is can nothrow move assignable");

   value_type _value;
   /*--:  *IMPLIMENT_FIELD*/
 protected:
   [[nodiscard]] constexpr auto get_if_detail() const noexcept
       -> detail_type const * {
      // NOTE: 防御策。unique_ptrの仕様ではnothrowとなっている。
      static_assert(noexcept(_value.get()), "can nothrow calling.");
      return _value.get();
   }

 public:
   constexpr error_detail() noexcept : _value(nullptr) {}
   error_detail(error_detail const &) = delete;
   constexpr error_detail(error_detail &&) noexcept = default;
   template <class... Args>
   // NOTE: `error_detail(error_detail const &)`と
   //       実質的に同じとならないようにする。
   //       value_typeがnothrow_constructibleであることも保証する。
   //       引数が0の際はデフォルトコンストラクタが呼ばれる。
      requires(!std::same_as<std::remove_cvref_t<Args>, error_detail> || ...) &&
              std::is_nothrow_constructible_v<value_type, Args...>
   constexpr error_detail(Args &&...args) noexcept
       : _value(std::forward<Args>(args)...) {}
   ~error_detail() = default;

   auto operator=(error_detail const &rhs) -> error_detail & = delete;
   auto operator=(error_detail &&rhs) -> error_detail & = default;
};

// [[detail_type]]
template <class D> class error_detail<std::monostate, D> {
 public: /*STRUCT_FIELD*/
   using detail_type = std::monostate;

 protected:
 private:
   using self_type = error_detail;

   /*--:  *IMPLIMENT_FIELD*/
 protected:
   [[nodiscard]] constexpr auto get_if_detail() const noexcept
       -> detail_type const * {
      return nullptr;
   }

 public:
   constexpr error_detail() noexcept = default;
   error_detail(error_detail const &) = delete;
   constexpr error_detail(error_detail &&) noexcept = default;
   ~error_detail() = default;

   auto operator=(error_detail const &rhs) -> error_detail & = delete;
   auto operator=(error_detail &&rhs) -> error_detail & = default;
};

// NOTE:
// enumを指定。成功値保持の際にスタック確保の負荷を少なくし、パフォーマンスを上げる。
// [[basic_error]]
template <class Code, class ErrDetail>
   requires std::is_enum_v<Code>
// Empty Base Optimization により detail_type == std::monostate
// の場合に追加ストレージを要求しないことを意図する。
class basic_error final : private ErrDetail {
 public: /*STRUCT_FIELD*/
   using code_type = Code;
   using detail_type = typename ErrDetail::detail_type;

 protected:
 private:
   using self_type = basic_error;

   code_type _code;
   /*--:  *IMPLIMENT_FIELD*/
 protected:
 public:
   basic_error() = delete;
   basic_error(basic_error const &) = delete;
   constexpr basic_error(basic_error &&) noexcept = default;
   /**
    * @brief エラー値の選択とErrDetail値の構築。
    * ErrDetail の構築方法そのものを利用者に委ね、
    * ErrDetailのコンストラクタがnoexceptであることを保証することで
    * 型構築時に例外の送出を無くす。
    * ErrDetailはstd::unique_ptrの構築がnoexceptであることを保証する。
    *
    * @tparam ArgsT std::unique_ptr 構築用の可変長引数型
    * @param code エラーコード(enum値)
    * @param args std::unique_ptr 構築用の可変長引数
    */
   template <class... Args>
      requires std::is_nothrow_constructible_v<ErrDetail, Args...>
   constexpr explicit basic_error(code_type code, Args &&...args) noexcept
       : ErrDetail(std::forward<Args>(args)...), _code(code) {}
   ~basic_error() = default;

   [[nodiscard]] constexpr auto code() const noexcept -> code_type {
      return _code;
   }
   /**
    * @brief 保持されている詳細情報へのポインタを取得する。
    *
    * 詳細情報を保持していない場合、または詳細情報がnullの場合は
    * nullptrを返す。
    */
   [[nodiscard]] constexpr auto get_if_detail() const noexcept
       -> detail_type const * {
      // NOTE: 防御策。ErrDetail(error_detailを期待)はnothrow修飾されている。
      static_assert(noexcept(ErrDetail::get_if_detail()),
                    "can nothrow calling.");
      return ErrDetail::get_if_detail();
   }

   constexpr auto operator==(code_type code) const noexcept -> bool {
      return code == _code;
   }

   auto operator=(basic_error const &rhs) -> basic_error & = delete;
   constexpr auto operator=(basic_error &&rhs) noexcept
       -> basic_error & = default;
};

/**
 * @brief error値を表現する型。
 * このクラスはenumによる表現できる。
 * また、そのenumに対応する説明用の型の表現も可能。
 *
 * 説明用の型はstd::monostate(デフォルトの指定型)の際は保持せず、
 * enum型を同じサイズとなることをライブラリ側が保証する。
 * そうでない場合はコンパイルエラーとなる。
 *
 * 説明用の型はstd::unique_ptrを利用し、ヒープに確保される。
 * デフォルトではnullptrを格納する。
 *
 * ヒープに確保できない場合は構築時の実装に委ねらる。
 * この型はstd::expectedなどによるエラー値処理を目的とするため、
 * デフォルト構築・コピー構築は不可とし、ムーブ構築のみ許可する。
 *
 * @tparam Code エラー表現用のenum型
 * @tparam ErrDetail エラー説明用の型
 * @tparam DetailDeleter 説明型のデリーター型
 */
export template <class Code, class Detail = std::monostate,
                 class DetailDeleter = std::default_delete<Detail>>
using error = basic_error<Code, error_detail<Detail, DetailDeleter>>;

/**
 * @brief error<Code, Detail, DetailDeleter> をエラー型とする
 * std::expected の結果型エイリアス。
 *
 * NOTE(v1.30案): result_traits（および value_type_with_extra による
 * std::variant 経由の複数モジュールエラー合成）は、現時点で利用実績が
 * ないため廃止した。複数モジュールをまたぐエラー合成が必要になった場合は
 * 別途改めて設計する（ライブラリ全体に影響するスコープのため一旦保留）。
 *
 * @tparam T 成功時の値型（std::expected の第1テンプレート引数）
 * @tparam Code エラー表現用のenum型
 * @tparam Detail エラー説明用の型
 * @tparam DetailDeleter 説明型のデリーター型
 */
export template <class T, class Code, class Detail = std::monostate,
                 class DetailDeleter = std::default_delete<Detail>>
using result = std::expected<T, error<Code, Detail, DetailDeleter>>;

/**
 * @brief error<Code, Detail, DetailDeleter> を保持する std::unexpected を
 * 構築するヘルパー関数。
 *
 * Detail・DetailDeleter をテンプレート引数として明示的に指定することで、
 * デリーター型を含めて error<Code, Detail, DetailDeleter> を直接構築し、
 * std::unexpected でラップして返す。
 *
 * Code は引数 code の型から自動的に推論される。一方 Detail・DetailDeleter は
 * 関数引数（Args...）からは推論できない（Args... は Detail そのものではなく、
 * 内部の std::unique_ptr<Detail, DetailDeleter> を構築するための引数である
 * ため）。Detail を保持したい場合、および独自のデリーターを使いたい場合は、
 * 呼び出し側でテンプレート引数として明示的に指定する必要がある
 * （例: make_unexpected<Code, int, MyDeleter>(code, ptr)）。
 *
 * requires節は basic_error 本体が要求する制約
 * （is_nothrow_constructible_v<ErrDetail, Args...>）とそのまま一致させて
 * おり、制約を満たさない呼び出しは basic_error 側と同じ理由で弾かれる。
 *
 * @tparam Code エラー表現用のenum型（引数 code から推論可能）
 * @tparam Detail エラー説明用の型（省略時 std::monostate）
 * @tparam DetailDeleter 説明型のデリーター型
 *         （省略時 std::default_delete<Detail>）
 * @tparam Args std::unique_ptr<Detail, DetailDeleter> 構築用の可変長引数型
 * @param code エラーコード(enum値)
 * @param args std::unique_ptr<Detail, DetailDeleter> 構築用の可変長引数
 */
export template <class Code, class Detail = std::monostate,
                 class DetailDeleter = std::default_delete<Detail>,
                 class... Args>
   requires std::is_enum_v<Code> &&
            std::is_nothrow_constructible_v<error_detail<Detail, DetailDeleter>,
                                            Args...>
[[nodiscard]] constexpr auto make_unexpected(Code code, Args &&...args) noexcept
    -> std::unexpected<error<Code, Detail, DetailDeleter>> {
   return std::unexpected<error<Code, Detail, DetailDeleter>>(
       std::in_place, code, std::forward<Args>(args)...);
}

enum class maboroutu_test_enum {};
using not_have_detail_type = error<maboroutu_test_enum>;
static_assert(sizeof(not_have_detail_type) == sizeof(maboroutu_test_enum),
              "is match enum size.");
using have_detail_type = error<maboroutu_test_enum, int>;
static_assert(sizeof(have_detail_type) != sizeof(maboroutu_test_enum),
              "is not match enum size.(has detail)");

static_assert(std::is_same_v<not_have_detail_type::detail_type, std::monostate>,
              "is monostate");
static_assert(
    std::is_same_v<
        decltype(std::declval<not_have_detail_type const &>().get_if_detail()),
        std::monostate const *>,
    "expect failed involk");

static_assert(std::is_same_v<have_detail_type::detail_type, int>, "is int");
static_assert(std::is_same_v<decltype(std::declval<have_detail_type const &>()
                                          .get_if_detail()),
                             int const *>,
              "expect can involk");

static_assert(std::is_same_v<result<int, maboroutu_test_enum>,
                             std::expected<int, not_have_detail_type>>,
              "result<T, Code> matches std::expected<T, error<Code>>.");

// make_unexpected() の型検証。decltype中のみで評価されるため、
// 実際の構築（ヒープ確保等）は発生しない。

// Detail省略時: Code はcode引数から推論され、Detailはstd::monostateとなる。
static_assert(std::is_same_v<decltype(make_unexpected(maboroutu_test_enum{})),
                             std::unexpected<not_have_detail_type>>,
              "make_unexpected(Code) matches std::unexpected<error<Code>>.");

// Detail・DetailDeleterを明示指定する例（デリーターにカスタム型を使用）。
struct maboroutu_test_detail_deleter {
   void operator()(int *p) const noexcept {
      // operator delete()が実装されていない可能性を踏まえて何もしない。
   }
};

static_assert(
    std::is_same_v<decltype(make_unexpected<maboroutu_test_enum, int,
                                            maboroutu_test_detail_deleter>(
                       maboroutu_test_enum{}, std::declval<int *>())),
                   std::unexpected<error<maboroutu_test_enum, int,
                                         maboroutu_test_detail_deleter>>>,
    "make_unexpected<Code, Detail, DetailDeleter> matches "
    "std::unexpected<error<Code, Detail, DetailDeleter>>.");

} // namespace maboroutu
