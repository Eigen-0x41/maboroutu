module;
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <span>
#include <vector>
export module maboroutu.memory_data_buffer;
export import maboroutu.data_buffer;
import maboroutu.core;
import maboroutu.error;
import maboroutu.data_source;

namespace maboroutu {

// [[memory_data_buffer]]
/**
 * @brief std::vector<std::byte> をバックエンドに持つ、data_buffer 準拠の
 *        メモリ上バイトバッファ。
 *
 * @note write()/append()/grow() による拡張はアロケーション失敗（bad_alloc）
 *       を除き失敗しない。
 */
export class memory_data_buffer {
 public: /*STRUCT_FIELD*/
 protected:
 private:
   using self_type = memory_data_buffer;
   using source_errc_type = data_source_result<void>::error_type::code_type;
   using buffer_errc_type = data_buffer_result<void>::error_type::code_type;

   std::vector<std::byte> _buf;

   [[nodiscard]] auto _in_range(this self_type const &self, region reg)
       -> bool {
      return reg.offset <= self._buf.size() &&
             reg.size <= self._buf.size() - reg.offset;
   }

   /*--:  *IMPLIMENT_FIELD*/
 public:
   memory_data_buffer() = default;
   explicit memory_data_buffer(std::size_t reserve_size) {
      _buf.reserve(reserve_size);
   }
   memory_data_buffer(memory_data_buffer const &) = default;
   memory_data_buffer(memory_data_buffer &&) = default;
   ~memory_data_buffer() = default;

   /**
    * @brief 現在保持しているバイト数を返す。
    */
   [[nodiscard]] auto size(this self_type const &self)
       -> data_source_result<std::size_t> {
      return self._buf.size();
   }

   /**
    * @brief region で指定した範囲を読み取る。
    *
    * @note data_source の短縮読み込み契約（library_spec.md v1.17）に従い、
    *       range が現在のサイズを超える場合は部分的なバイト列を返さず
    *       out_of_range を返す。
    */
   [[nodiscard]] auto read(this self_type const &self, region reg)
       -> data_source_result<byte_array> {
      if (!self._in_range(reg)) [[unlikely]] {
         return make_unexpected(source_errc_type::out_of_range);
      }
      byte_array ret_value{
          .value = std::make_unique<decltype(ret_value)::value_type>(reg.size),
          .size = reg.size,
      };
      std::ranges::copy_n(self._buf.begin() + reg.offset, reg.size,
                          ret_value.value.get());
      return ret_value;
   }

   /**
    * @brief region で指定した範囲へ書き込む。
    *
    * @note range が現在のサイズを超える場合の挙動は実装定義
    *       （library_spec.md 4.3節）。本実装は自動的に拡張するため、
    *       アロケーション失敗を除き失敗しない。
    * @pre data.size() == r.size（呼び出し側の責務。不一致時はUB）
    */
   [[nodiscard]] auto write(this self_type &self, region reg,
                            std::span<std::byte const> data)
       -> data_source_result<void> {
      assert(data.size() == reg.size);
      if (auto const required_end = reg.offset + reg.size;
          required_end > self._buf.size()) [[unlikely]] {
         self._buf.resize(required_end);
      }
      std::ranges::copy(data, self._buf.begin() + reg.offset);
      return {};
   }

   /**
    * @brief 末尾に data を追記し、書き込まれた region を返す。
    */
   [[nodiscard]] auto append(this self_type &self,
                             std::span<std::byte const> data)
       -> data_buffer_result<region> {
      auto const offset = self._buf.size();
      self._buf.insert(self._buf.end(), data.begin(), data.end());
      return region{
          .offset = offset,
          .size = data.size(),
      };
   }

   /**
    * @brief 末尾に n バイト分の領域を（0埋めで）予約し、その region を返す。
    *
    * @note 予約のみで内容は未定（0埋め）。パッチ用プレースホルダとして使う。
    */
   [[nodiscard]] auto grow(this self_type &self, std::size_t n)
       -> data_buffer_result<region> {
      auto const offset = self._buf.size();
      self._buf.resize(self._buf.size() + n);
      return region{
          .offset = offset,
          .size = n,
      };
   }

   /**
    * @brief region の範囲を、コピーを伴わず直接参照する。
    *
    * @note 返す std::span は self の生存期間・再確保（append/grow/write
    *       による再確保）の影響を受ける非所有ビューである。呼び出し側は
    *       これらの操作の前に使い終える責任を負う。
    */
   [[nodiscard]] auto view(this self_type &self, region reg)
       -> data_buffer_result<std::span<std::byte>> {
      if (!self._in_range(reg)) [[unlikely]] {
         return make_unexpected(buffer_errc_type::out_of_range);
      }
      return std::span<std::byte>{self._buf.data() + reg.offset, reg.size};
   }

   auto operator=(memory_data_buffer const &) -> memory_data_buffer & = default;
   auto operator=(memory_data_buffer &&) -> memory_data_buffer & = default;
};
static_assert(data_buffer<memory_data_buffer>, "");
static_assert(std::is_constructible_v<data_source_handle, memory_data_buffer &>,
              "");
static_assert(
    std::is_constructible_v<writable_data_source_handle, memory_data_buffer &>,
    "");

} // namespace maboroutu
