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
// NOTE: data_source concept（ランダムアクセス前提）を土台とするため、
// 任意offsetへの seek() は意図的に公開しない（一方向にのみ前進する）。
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
         return std::move(*result);
      }

      return convert_from_data_source_result_error_type(result);
   }

   /**
    * @brief size バイトを、確保を伴わずに dst の先頭へ読み取る。
    *
    * @pre dst.size() >= size（呼び出し側の責務。違反は assert で検出）
    * @note 失敗時は count() を進めない。
    */
   [[nodiscard]] auto read_into(std::size_t size, std::span<std::byte> dst)
       -> result_type<void> {
      auto result = _src->read_into(
          region{
              .offset = _count,
              .size = size,
          },
          dst);
      if (result) [[likely]] {
         _count += size;
         return {};
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

   // 読み書きを伴わずに、累積バイト数のみを進める。
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
