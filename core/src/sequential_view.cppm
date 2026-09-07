module;
#include <array>
#include <cstddef>
#include <expected>
#include <span>
#include <vector>
export module maboroutu.sequential_view;
export import maboroutu.core;
export import maboroutu.error;
export import maboroutu.data_source;
export import maboroutu.binary_convert;

namespace maboroutu {

// data_source を非所有で参照し、これまでに成功した読み書きの累積バイト数
// (_count) のみを状態として保持する薄いビュー。
//
// NOTE: 現状は data_source concept（ランダムアクセス前提）を土台とする
// ため、任意offsetへの seek() は意図的に公開しない。将来、真にフォワード
// オンリーな入力（例: 非シーク可能なストリーム）向けの別conceptを導入する
// 場合にも、このインターフェース（一方向にのみ前進する）のまま矛盾なく
// 拡張できることを意図した設計。
export template <data_source Src> struct sequential_view {
 private:
   Src &_src;
   std::size_t _count = 0; // これまでに成功した読み書きの累積バイト数

 public:
   sequential_view() = delete;
   explicit sequential_view(Src &src, std::size_t initial_count = 0) noexcept
       : _src(src), _count(initial_count) {}

   [[nodiscard]] auto count(this sequential_view const &self) noexcept
       -> std::size_t {
      return self._count;
   }

   // n バイトを読み飛ばしたものとして扱う。
   // NOTE: 現状（data_source限定のスコープ）では _count を進めるだけで
   // 成立するが、これは Src が実際にはランダムアクセス可能だから
   // 偶然成立しているに過ぎない。将来フォワードオンリーな Src を
   // 受け入れる場合、この実装は「n バイトを実際に読んで捨てる」処理へ
   // 変更する必要がある。
   auto skip(this sequential_view &self, std::size_t n) noexcept -> void {
      self._count += n;
   }

   [[nodiscard]] auto size(this sequential_view const &self) {
      return self._src.size();
   }
   [[nodiscard]] auto remaining(this sequential_view const &self)
       -> data_source_result<std::size_t> {
      auto total = self.size();
      if (!total) {
         return std::unexpected(total.error());
      }
      return *total - self._count;
   }

   // 以下、_src への直接アクセスを許可する friend 宣言。
   // 各自由関数の実際のテンプレートパラメータ列・戻り値型と一致させる
   // 必要があるため、8関数それぞれに個別宣言する。
   // クラス自身のテンプレート引数 Src とのシャドーイングを避けるため、
   // friend宣言側のデータソース型引数は S とする。
   template <data_source S>
   friend auto read(sequential_view<S> &, std::size_t)
       -> data_source_result<byte_array>;
   template <writable_data_source S>
   friend auto write(sequential_view<S> &, std::span<std::byte const>)
       -> data_source_result<void>;
   template <endian Endian, numberable T, data_source S>
   friend auto read_value(sequential_view<S> &) -> data_source_result<T>;
   template <endian Endian, numberable T, writable_data_source S>
   friend auto write_value(sequential_view<S> &, T)
       -> data_source_result<void>;
   template <endian Endian, numberable T, std::size_t Size, data_source S>
   friend auto read_array(sequential_view<S> &)
       -> data_source_result<std::array<T, Size>>;
   template <endian Endian, numberable T, std::size_t Size,
             writable_data_source S>
   friend auto write_array(sequential_view<S> &, std::array<T, Size> const &)
       -> data_source_result<void>;
   template <endian Endian, numberable T, data_source S>
   friend auto read_vector(sequential_view<S> &, std::size_t)
       -> data_source_result<std::vector<T>>;
   template <endian Endian, numberable T, writable_data_source S>
   friend auto write_vector(sequential_view<S> &, std::vector<T> const &)
       -> data_source_result<void>;
};

// --- 生バイト列の読み書き（binary_convert非依存、data_sourceのみに依存） ---

export template <data_source Src>
[[nodiscard]] auto read(sequential_view<Src> &src, std::size_t size)
    -> data_source_result<byte_array> {
   auto result = src._src.read(region{
       .offset = src.count(),
       .size = size,
   });
   if (result) {
      src.skip(size);
   }
   return result;
}

export template <writable_data_source Src>
[[nodiscard]] auto write(sequential_view<Src> &dst,
                         std::span<std::byte const> data)
    -> data_source_result<void> {
   auto result = dst._src.write(
       region{
           .offset = dst.count(),
           .size = data.size(),
       },
       data);
   if (result) {
      dst.skip(data.size());
   }
   return result;
}

// --- binary_convert と同名のオーバーロード（薄い委譲） ---

export template <endian Endian, numberable T, data_source Src>
[[nodiscard]] auto read_value(sequential_view<Src> &src)
    -> data_source_result<T> {
   auto result = maboroutu::read_value<Endian, T>(src._src, src.count());
   if (result) {
      src.skip(sizeof(T));
   }
   return result;
}

export template <endian Endian, numberable T, writable_data_source Src>
[[nodiscard]] auto write_value(sequential_view<Src> &dst, T value)
    -> data_source_result<void> {
   auto result = maboroutu::write_value<Endian, T>(dst._src, dst.count(), value);
   if (result) {
      dst.skip(sizeof(T));
   }
   return result;
}

export template <endian Endian, numberable T, std::size_t Size, data_source Src>
[[nodiscard]] auto read_array(sequential_view<Src> &src)
    -> data_source_result<std::array<T, Size>> {
   auto result = maboroutu::read_array<Endian, T, Size>(src._src, src.count());
   if (result) {
      src.skip(sizeof(T) * Size);
   }
   return result;
}

export template <endian Endian, numberable T, std::size_t Size,
                 writable_data_source Src>
[[nodiscard]] auto write_array(sequential_view<Src> &dst,
                               std::array<T, Size> const &values)
    -> data_source_result<void> {
   auto result =
       maboroutu::write_array<Endian, T, Size>(dst._src, dst.count(), values);
   if (result) {
      dst.skip(sizeof(T) * Size);
   }
   return result;
}

export template <endian Endian, numberable T, data_source Src>
[[nodiscard]] auto read_vector(sequential_view<Src> &src, std::size_t count)
    -> data_source_result<std::vector<T>> {
   auto result =
       maboroutu::read_vector<Endian, T>(src._src, src.count(), count);
   if (result) {
      src.skip(sizeof(T) * count);
   }
   return result;
}

export template <endian Endian, numberable T, writable_data_source Src>
[[nodiscard]] auto write_vector(sequential_view<Src> &dst,
                                std::vector<T> const &values)
    -> data_source_result<void> {
   auto result =
       maboroutu::write_vector<Endian, T>(dst._src, dst.count(), values);
   if (result) {
      dst.skip(sizeof(T) * values.size());
   }
   return result;
}

} // namespace maboroutu
