module;
#include <array>
#include <cassert>
#include <climits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <type_traits>
#include <utility>
export module maboroutu.binary_layout;
export import maboroutu.core;
export import maboroutu.error;
import maboroutu.data_source;
import maboroutu.data_buffer;

namespace maboroutu {

static_assert(CHAR_BIT == 8, "1byte は 8bit である必要があります。");
template <std::size_t Bytes, class T, class FalseT>
using size_conditional_t =
    typename std::conditional_t<Bytes <= sizeof(T), T, FalseT>;

// Bytes バイト分の符号なし整数値を保持できる、最小の標準整数型を選ぶ。
// numberable（binary_convert, sizeof(T)固定）の枠に収まらない任意幅
// （FLACの24bit長フィールド等）を表現するための内部実装詳細。
template <std::size_t Bytes> struct uint_storage {
   static_assert(Bytes >= 1 && Bytes <= sizeof(std::uint64_t),
                 "Bytes は 1 以上 8 以下でなければならない");
   using type = size_conditional_t<
       Bytes, std::uint8_t,
       size_conditional_t<
           Bytes, std::uint16_t,
           size_conditional_t<Bytes, std::uint32_t, std::uint64_t>>>;
};
template <std::size_t Bytes>
using uint_storage_t = typename uint_storage<Bytes>::type;

/**
 * @brief Bytes バイト（1〜8、標準整数幅に限らない任意幅）の符号なし整数を
 *        data_source から読み取る。
 *
 * @note FLACの24bit長フィールド等、numberable（binary_convert、
 *       sizeof(T)固定）の枠に収まらない幅を表現するためのプリミティブ。
 *       data_source::read()の短縮読み込み契約（v1.17）に従い、範囲外は
 *       out_of_range として伝播する。
 */
export template <std::size_t Bytes, endian Endian, data_source Src>
[[nodiscard]] auto read_uint(Src &src, std::size_t offset)
    -> data_source_result<uint_storage_t<Bytes>> {
   auto result = src.read(region{
       .offset = offset,
       .size = Bytes,
   });
   if (!result) [[unlikely]] {
      return std::unexpected(std::move(result).error());
   }

   using storage_type = uint_storage_t<Bytes>;
   storage_type value{};
#pragma unroll sizeof(std::uint16_t)
   for (std::size_t i = 0; i < Bytes; ++i) {
      static_assert(endian::native == endian::big ||
                        endian::native == endian::little,
                    "not supported endian.");
      auto const byte_index = (Endian == endian::little) ? i : (Bytes - 1 - i);
      value |= static_cast<storage_type>(
                   static_cast<std::uint8_t>(result->value[byte_index]))
               << (CHAR_BIT * i);
   }
   return value;
}

/**
 * @brief Bytes バイト（1〜8、標準整数幅に限らない任意幅）の符号なし整数を
 *        writable_data_source へ書き込む。
 */
export template <std::size_t Bytes, endian Endian, writable_data_source Src>
[[nodiscard]] auto write_uint(Src &dst, std::size_t offset,
                              uint_storage_t<Bytes> value)
    -> data_source_result<void> {
   std::array<std::byte, Bytes> bytes{};
#pragma unroll sizeof(std::uint16_t)
   for (std::size_t i = 0; i < Bytes; ++i) {
      static_assert(endian::native == endian::big ||
                        endian::native == endian::little,
                    "not supported endian.");
      auto const byte_index = (Endian == endian::little) ? i : (Bytes - 1 - i);
      bytes[byte_index] = static_cast<std::byte>((value >> (CHAR_BIT * i)) &
                                                 uint_storage_t<Bytes>{0xFF});
   }
   return dst.write(
       region{
           .offset = offset,
           .size = Bytes,
       },
       bytes);
}

/**
 * @brief data_buffer 上に予約したプレースホルダを、後から確定値で上書き
 *        （パッチ）するためのRAIIハンドル。
 *
 * @note バイトオフセットへの参照・テーブルインデックスへの参照のいずれ
 *       にも使える（値の意味は呼び出し側が決める。application_spec.md
 *       検討時の分類 (b)/(c) 双方の下位機構として共用する）。
 * @note スコープを抜けるまでに resolve()（または明示的に abandon()）を
 *       呼ばなければ、デストラクタで assert により検出する（呼び忘れは
 *       バグとして扱う。7章 (b) スコープ完結型パッチの設計）。
 * @note コピー不可・ムーブのみ。二重解決を防ぐため、ムーブ元は
 *       「解決済み」扱いに遷移する。
 * @note 予約領域のサイズは常に Bytes（テンプレート引数）と一致するため、
 *       region ではなく std::size_t（先頭位置のみ）を保持する。
 */
export template <std::size_t Bytes, endian Endian, data_buffer Buffer>
// [[position_patch]]
class position_patch {
 public: /*STRUCT_FIELD*/
   using storage_type = uint_storage_t<Bytes>;

 protected:
 private:
   using self_type = position_patch;

   Buffer *_buf = nullptr;
   std::size_t _position = 0;
   bool _resolved = false;

   /*--:  *IMPLIMENT_FIELD*/
 protected:
 public:
   position_patch() = delete;
   position_patch(position_patch const &) = delete;
   position_patch(position_patch &&other) noexcept
       : _buf(other._buf), _position(other._position),
         _resolved(other._resolved) {
      other._buf = nullptr;
      other._resolved = true;
   }
   position_patch(Buffer &buf, std::size_t position) noexcept
       : _buf(&buf), _position(position) {}
   ~position_patch() {
      assert(_resolved &&
             "position_patch が resolve()/abandon() されずに破棄されました");
   }

   [[nodiscard]] auto position(this self_type const &self) -> std::size_t {
      return self._position;
   }

   [[nodiscard]] auto resolved(this self_type const &self) -> bool {
      return self._resolved;
   }

   /**
    * @brief 確定値を書き込み、パッチを解決する。
    * @pre 未解決であること（二重解決はUB、assertで検出）。
    */
   auto resolve(this self_type &self, storage_type value)
       -> data_source_result<void> {
      assert(!self._resolved && "position_patch が二重に解決されました");
      assert(self._buf != nullptr);
      auto result =
          write_uint<Bytes, Endian>(*self._buf, self._position, value);
      self._resolved = true;
      return result;
   }

   /**
    * @brief 意図的に解決せず放棄する（呼び出し元で別のエラー処理により
    *        全体を中断する場合等）。プレースホルダの0埋め内容はそのまま
    *        残る。
    */
   auto abandon(this self_type &self) -> void { self._resolved = true; }

   auto operator=(position_patch const &other) -> position_patch & = default;
   auto operator=(position_patch &&other) noexcept -> position_patch & {
      if (this == &other) {
         return *this;
      }
      assert(_resolved && "未解決の position_patch が上書きされました");
      _buf = other._buf;
      _position = other._position;
      _resolved = other._resolved;
      other._buf = nullptr;
      other._resolved = true;
      return *this;
   }
};

/**
 * @brief data_buffer の末尾に Bytes バイト分のプレースホルダ（0埋め）を
 *        予約し、position_patch を返す。
 */
export template <std::size_t Bytes, endian Endian, data_buffer Buffer>
[[nodiscard]] auto reserve_patch(Buffer &buf)
    -> data_buffer_result<position_patch<Bytes, Endian, Buffer>> {
   auto grown = buf.grow(Bytes);
   if (!grown) [[unlikely]] {
      return std::unexpected(std::move(grown).error());
   }
   return position_patch<Bytes, Endian, Buffer>{buf, grown->offset};
}

} // namespace maboroutu
