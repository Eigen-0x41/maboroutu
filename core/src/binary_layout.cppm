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
 * @brief テーブル引き参照（index_ref）。TableTag により、異なるテーブルの
 *        添字同士を型で区別する。参照先はテーブルの要素数が確定すれば
 *        解決可能であり、レイアウト全体の確定（バックパッチ）を待たない
 *        （application_spec.md 検討時の分類 (a)/(A) に相当）。
 */
export template <class TableTag> struct index_ref {
   std::size_t index;
};

/**
 * @brief data_buffer 上に予約したプレースホルダを、後から確定値で上書き
 *        （パッチ）するためのRAIIハンドル。
 *
 * @note スコープを抜けるまでに resolve()（または明示的に abandon()）を
 *       呼ばなければ、デストラクタで assert により検出する（呼び忘れは
 *       バグとして扱う。7章 (b) スコープ完結型パッチの設計）。
 * @note コピー不可・ムーブのみ。二重解決を防ぐため、ムーブ元は
 *       「解決済み」扱いに遷移する。
 */
export template <std::size_t Bytes, endian Endian, data_buffer Buffer>
// [[offset_patch]]
class offset_patch {
 public: /*STRUCT_FIELD*/
   using storage_type = uint_storage_t<Bytes>;

 protected:
 private:
   using self_type = offset_patch;

   Buffer *_buf = nullptr;
   region _placeholder{};
   bool _resolved = false;

   /*--:  *IMPLIMENT_FIELD*/
 protected:
 public:
   offset_patch() = delete;
   offset_patch(offset_patch const &) = delete;
   offset_patch(offset_patch &&other) noexcept
       : _buf(other._buf), _placeholder(other._placeholder),
         _resolved(other._resolved) {
      other._buf = nullptr;
      other._resolved = true;
   }
   offset_patch(Buffer &buf, region placeholder) noexcept
       : _buf(&buf), _placeholder(placeholder) {}
   ~offset_patch() {
      assert(_resolved &&
             "offset_patch が resolve()/abandon() されずに破棄されました");
   }

   [[nodiscard]] auto placeholder(this self_type const &self) -> region {
      return self._placeholder;
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
      assert(!self._resolved && "offset_patch が二重に解決されました");
      assert(self._buf != nullptr);
      auto result = write_uint<Bytes, Endian>(*self._buf,
                                              self._placeholder.offset, value);
      self._resolved = true;
      return result;
   }

   /**
    * @brief 意図的に解決せず放棄する（呼び出し元で別のエラー処理により
    *        全体を中断する場合等）。プレースホルダの0埋め内容はそのまま
    *        残る。
    */
   auto abandon(this self_type &self) -> void { self._resolved = true; }

   auto operator=(offset_patch const &other) -> offset_patch & = default;
   auto operator=(offset_patch &&other) noexcept -> offset_patch & {
      if (this == &other) {
         return *this;
      }
      assert(_resolved && "未解決の offset_patch が上書きされました");
      _buf = other._buf;
      _placeholder = other._placeholder;
      _resolved = other._resolved;
      other._buf = nullptr;
      other._resolved = true;
      return *this;
   }
};

/**
 * @brief data_buffer の末尾に Bytes バイト分のプレースホルダ（0埋め）を
 *        予約し、offset_patch を返す。
 */
export template <std::size_t Bytes, endian Endian, data_buffer Buffer>
[[nodiscard]] auto reserve_patch(Buffer &buf)
    -> data_buffer_result<offset_patch<Bytes, Endian, Buffer>> {
   auto grown = buf.grow(Bytes);
   if (!grown) [[unlikely]] {
      return std::unexpected(std::move(grown).error());
   }
   return offset_patch<Bytes, Endian, Buffer>{buf, *grown};
}

// data_source由来のエラーを data_buffer のエラードメインへ変換する
// （sequential_view.cppm の convert_from_data_source_result_error_type
//   と同種の、モジュール間エラー変換）。
template <class ResultT>
[[nodiscard]] auto
_convert_source_error_to_buffer_error(ResultT const &result) {
   auto const code = result.error().code();
   using source_code_type = decltype(code);
   using buffer_code_type = data_buffer_result<void>::error_type::code_type;
   if (code == source_code_type::out_of_range) {
      return make_unexpected(buffer_code_type::out_of_range);
   }
   return make_unexpected(buffer_code_type::operation_failure);
}

/**
 * @brief TLV(Tag-Length-Value)レコード1つを読み取った結果。
 *
 * @note リストの終端判定（親の長さで区切る／センチネルで区切る／タグに
 *       埋め込まれたフラグビットで区切る等、フォーマットごとに異なる。）
 *       は本関数の責務外とする。呼び出し側が record から次のオフセット
 *       （record.offset + record.size）を計算し、判定ポリシーを適用する。
 */
export template <class TagT> struct tlv_record {
   TagT tag;
   region payload; // ペイロードの位置。生バイト列は src.read(payload) で取得
   region record;  // tag+length+payload 全体の位置（次レコードへの前進に使う）
};

/**
 * @brief data_source から TLV レコード1つを読み取る。
 */
export template <std::size_t TagBytes, std::size_t LengthBytes, endian Endian,
                 class TagT, data_source Src>
   requires std::is_enum_v<TagT> || std::unsigned_integral<TagT>
[[nodiscard]] auto read_tlv_record(Src &src, std::size_t offset)
    -> data_source_result<tlv_record<TagT>> {
   auto tag_read = read_uint<TagBytes, Endian>(src, offset);
   if (!tag_read) [[unlikely]] {
      return std::unexpected(std::move(tag_read).error());
   }
   auto length_read = read_uint<LengthBytes, Endian>(src, offset + TagBytes);
   if (!length_read) [[unlikely]] {
      return std::unexpected(std::move(length_read).error());
   }

   auto const payload_offset = offset + TagBytes + LengthBytes;
   return tlv_record<TagT>{
       .tag = static_cast<TagT>(*tag_read),
       .payload =
           region{
               .offset = payload_offset,
               .size = *length_read,
           },
       .record =
           region{
               .offset = offset,
               .size = TagBytes + LengthBytes + *length_read,
           },
   };
}

/**
 * @brief data_buffer の末尾へ TLV レコード1つを書き込む。
 */
export template <std::size_t TagBytes, std::size_t LengthBytes, endian Endian,
                 class TagT, data_buffer Buffer>
   requires std::is_enum_v<TagT> || std::unsigned_integral<TagT>
[[nodiscard]] auto write_tlv_record(Buffer &buf, TagT tag,
                                    std::span<std::byte const> payload)
    -> data_buffer_result<region> {
   auto header = buf.grow(TagBytes + LengthBytes);
   if (!header) [[unlikely]] {
      return std::unexpected(std::move(header).error());
   }

   auto tag_written = write_uint<TagBytes, Endian>(
       buf, header->offset, static_cast<uint_storage_t<TagBytes>>(tag));
   if (!tag_written) [[unlikely]] {
      return _convert_source_error_to_buffer_error(tag_written);
   }
   auto length_written = write_uint<LengthBytes, Endian>(
       buf, header->offset + TagBytes,
       static_cast<uint_storage_t<LengthBytes>>(payload.size()));
   if (!length_written) [[unlikely]] {
      return _convert_source_error_to_buffer_error(length_written);
   }

   auto payload_region = buf.append(payload);
   if (!payload_region) [[unlikely]] {
      return std::unexpected(std::move(payload_region).error());
   }

   return region{
       .offset = header->offset,
       .size = TagBytes + LengthBytes + payload.size(),
   };
}

} // namespace maboroutu
