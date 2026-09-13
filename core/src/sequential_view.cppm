module;
#include <cstddef>
#include <expected>
#include <span>
#include <utility>
export module maboroutu.sequential_view;
export import maboroutu.core;
export import maboroutu.error;

export import maboroutu.binary_convert;
export import maboroutu.data_source;
export import maboroutu.sequential_source;

namespace maboroutu {

// data_source を非所有で参照し、これまでに成功した読み書きの累積バイト数
// (_count) のみを状態として保持する薄いビュー。
//
// NOTE: 現状は data_source concept（ランダムアクセス前提）を土台とする
// ため、任意offsetへの seek() は意図的に公開しない。将来、真にフォワード
// オンリーな入力（例: 非シーク可能なストリーム）向けの別concept
// を導入する場合にも、このインターフェース（一方向にのみ前進する）のまま
// 矛盾なく拡張できることを意図した設計。
export template <data_source DataSource>
// [[sequential_view]]
class sequential_view {
 public: /*STRUCT_FIELD*/
   using value_type = DataSource;

 protected:
 private:
   using self_type = sequential_view;
   template <class T> using result_type = sequential_source_result<T>;

   value_type *_src;
   std::size_t _count = 0;

   /*--:  *IMPLIMENT_FIELD*/
 protected:
   template <class ResultT>
   auto convert_from_data_source_result_error_type(ResultT const &result) {
      auto code = result.error().code();
      using code_type = decltype(code);
      switch (code) {
      case code_type::out_of_range:
         return make_unexpected(
             result_type<void>::error_type::code_type::out_of_range);
      case code_type::operation_failure:
         return make_unexpected(
             result_type<void>::error_type::code_type::operation_failure);
      default:
         break;
      }
      return make_unexpected(
          result_type<void>::error_type::code_type::invalid_member_variable);
   }

 public:
   sequential_view() = delete;
   sequential_view(sequential_view const &) = delete;
   sequential_view(sequential_view &&) = default;
   explicit sequential_view(value_type &src, std::size_t initial_count = 0)
       : _src(&src), _count(initial_count) {}
   ~sequential_view() = default;

   [[nodiscard]] auto count() const noexcept -> std::size_t { return _count; }
   [[nodiscard]] auto read(std::size_t rdsize) -> result_type<byte_array> {
      auto result = _src->read(region{
          .offset = _count,
          .size = rdsize,
      });
      if (result) [[likely]] {
         _count += rdsize;
         return *result;
      }

      return convert_from_data_source_result_error_type(result);
   }

   template <class Self>
      requires writable_data_source<typename Self::value_type>
   [[nodiscard]] auto write(this Self &self, std::span<std::byte const> data)
       -> result_type<void> {
      auto result = self._src->write(
          region{
              .offset = self._count,
              .size = data.size(),
          },
          data);
      if (result) [[likely]] {
         self._count += data.size();
         return {};
      }
      return self.convert_from_data_source_result_error_type(result);
   }

   // NOTE: オプション実装
   template <class Self>
   auto skip(this Self &self, std::size_t n) noexcept -> void {
      self._count += n;
   }

   auto operator=(sequential_view const &rhs) -> sequential_view & = delete;
   auto operator=(sequential_view &&rhs) -> sequential_view & = default;
};
static_assert(sequential_source<sequential_view<null_data_source>>, "");
static_assert(
    writable_sequential_source<sequential_view<null_writable_data_source>>, "");

} // namespace maboroutu
