module;
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <forward_list>
#include <limits>
#include <memory>
#include <numeric>
#include <span>
#include <tuple>
#include <utility>
#include <vector>
export module maboroutu.memory_data_buffer;
export import maboroutu.data_buffer;
import maboroutu.core;
import maboroutu.error;

namespace maboroutu {

template <class T>
constexpr std::size_t table_block_size =
    std::lcm(sizeof(T), recommended_alignment) / sizeof(T);

// [[memory_data_buffer]]
/**
 * @brief std::vector<std::byte> をバックエンドに持つ、data_buffer 準拠の
 *        メモリ上バイトバッファ。
 *
 * @note write()/append()/grow() による拡張はアロケーション失敗（bad_alloc）
 *       を除き失敗しない。
 */
export class memory_data_buffer {
   static_assert(std::has_single_bit(table_block_size<std::byte>), "");
   template <template <class, std::size_t> class Container>
   using unit_traits = Container<std::byte, table_block_size<std::byte>>;
   template <template <class, std::size_t> class Container>
   using const_unit_traits =
       Container<std::byte const, table_block_size<std::byte>>;

 public: /*STRUCT_FIELD*/
   using view_type = unit_traits<segmented_span>;
   using const_view_type = const_unit_traits<segmented_span>;

 protected:
 private:
   using self_type = memory_data_buffer;
   using source_errc_type = data_source_result<void>::error_type::code_type;
   using buffer_errc_type = data_buffer_result<void>::error_type::code_type;

   using unit_type = unit_traits<std::array>;
   using unit_span_type = unit_traits<std::span>;
   using chunk_type = std::vector<unit_span_type>;

   std::forward_list<unit_type> _unit_manager;
   std::vector<unit_span_type> _data;
   std::size_t _size = 0;

   /*--:  *IMPLIMENT_FIELD*/
   [[nodiscard]] auto _in_range(this self_type const &self, region reg)
       -> bool {
      return (reg.offset <= self._size) &&
             (reg.size <= self._size - reg.offset);
   }

   // _resize() のブロック数の算出（切り上げ）が桁あふれしない範囲か。
   // 範囲外のサイズでは、ブロック数が過小に算出され、_size だけが
   // 大きい不整合な状態になる。
   [[nodiscard]] static constexpr auto _can_resize(std::size_t size) noexcept
       -> bool {
      return size <= std::numeric_limits<std::size_t>::max() -
                         (std::tuple_size_v<unit_type> - 1);
   }

   void _resize(std::size_t size) {
      auto const required_size = (size + std::tuple_size_v<unit_type> - 1) /
                                 std::tuple_size_v<unit_type>;
      auto const cmp = required_size <=> _data.size();
      if (cmp > 0) {
         auto const diff = required_size - _data.size();
         for (std::size_t i = 0; i < diff; i++) {
            _unit_manager.push_front({});
            _data.emplace_back(_unit_manager.front());
         }
      } else if (cmp < 0) {
         auto const diff = _data.size() - required_size;
         _data.erase(_data.end() - static_cast<std::ptrdiff_t>(diff),
                     _data.end());
         for (std::size_t i = 0; i < diff; i++) {
            _unit_manager.pop_front();
         }
      }
      _size = size;
   }

 public:
   memory_data_buffer() = default;
   memory_data_buffer(memory_data_buffer const &value)
       : _unit_manager(value._unit_manager), _size(value._size) {
      _data.reserve(value._data.size());
      for (auto &unit : _unit_manager) {
         _data.emplace_back(unit);
      }
      std::ranges::reverse(_data);
   }
   /**
    * @brief ムーブ元は空（size() == 0）になり、そのまま再利用できる。
    * @note ユニットは std::forward_list が所有するためノードの
    *       アドレスは維持され、_data の std::span は有効なままである。
    */
   memory_data_buffer(memory_data_buffer &&other) noexcept
       : _unit_manager(std::move(other._unit_manager)),
         _data(std::move(other._data)),
         _size(std::exchange(other._size, 0)) {
      other._unit_manager.clear();
      other._data.clear();
   }
   ~memory_data_buffer() = default;

   /**
    * @brief 現在保持しているバイト数を返す。
    */
   [[nodiscard]] auto size(this self_type const &self)
       -> data_source_result<std::size_t> {
      return self._size;
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
      const_view_type view(self._data, reg);
      std::ranges::copy(view, ret_value.value.get());
      return ret_value;
   }

   /**
    * @brief region で指定した範囲へ書き込む。
    *
    * @note range が現在のサイズを超える場合の挙動は実装定義
    *       （library_spec.md 4.3節）。本実装は自動的に拡張するため、
    *       アロケーション失敗を除き失敗しない。
    * @pre data.size() == r.size（呼び出し側の責務。不一致時はUB）
    * @note offset + size が size_t を超える場合は out_of_range を返し、
    *       バッファは変更しない。
    */
   [[nodiscard]] auto write(this self_type &self, region reg,
                            std::span<std::byte const> data)
       -> data_source_result<void> {
      assert(data.size() == reg.size);
      if (reg.size > std::numeric_limits<std::size_t>::max() - reg.offset)
          [[unlikely]] {
         return make_unexpected(source_errc_type::out_of_range);
      }
      if (auto const required_end = reg.offset + reg.size;
          required_end > self._size) [[unlikely]] {
         if (!_can_resize(required_end)) [[unlikely]] {
            return make_unexpected(source_errc_type::out_of_range);
         }
         self._resize(required_end);
      }
      view_type view(self._data, reg);
      std::ranges::copy(data, view.begin());
      return {};
   }

   /**
    * @brief 末尾に data を追記し、書き込まれた region を返す。
    */
   [[nodiscard]] auto append(this self_type &self,
                             std::span<std::byte const> data)
       -> data_buffer_result<region> {
      if (data.size() > std::numeric_limits<std::size_t>::max() - self._size)
          [[unlikely]] {
         return make_unexpected(buffer_errc_type::out_of_range);
      }
      if (!_can_resize(self._size + data.size())) [[unlikely]] {
         return make_unexpected(buffer_errc_type::out_of_range);
      }
      auto const offset = self._size;

      auto const result = region{
          .offset = offset,
          .size = data.size(),
      };

      self._resize(self._size + data.size());
      view_type view(self._data, result);
      std::ranges::copy(data, view.begin());

      return result;
   }

   /**
    * @brief 末尾に n バイト分の領域を（0埋めで）予約し、その region を返す。
    *
    * @note 予約のみで内容は未定（0埋め）。パッチ用プレースホルダとして使う。
    */
   [[nodiscard]] auto grow(this self_type &self, std::size_t n)
       -> data_buffer_result<region> {
      if (n > std::numeric_limits<std::size_t>::max() - self._size)
          [[unlikely]] {
         return make_unexpected(buffer_errc_type::out_of_range);
      }
      if (!_can_resize(self._size + n)) [[unlikely]] {
         return make_unexpected(buffer_errc_type::out_of_range);
      }
      auto const offset = self._size;
      self._resize(self._size + n);
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
       -> data_buffer_result<view_type> {
      if (!self._in_range(reg)) [[unlikely]] {
         return make_unexpected(buffer_errc_type::out_of_range);
      }
      return view_type(self._data, reg);
   }

   auto operator=(memory_data_buffer const &value) -> memory_data_buffer & {
      if (this == &value) [[unlikely]] {
         return *this;
      }
      _resize(value._size);
      std::ranges::copy(value._unit_manager, _unit_manager.begin());
      return *this;
   }
   auto operator=(memory_data_buffer &&other) noexcept -> memory_data_buffer & {
      if (this != &other) {
         _unit_manager = std::move(other._unit_manager);
         _data = std::move(other._data);
         _size = std::exchange(other._size, 0);
         other._unit_manager.clear();
         other._data.clear();
      }
      return *this;
   }
};
static_assert(data_buffer<memory_data_buffer>, "");
static_assert(std::is_constructible_v<data_source_handle, memory_data_buffer &>,
              "");
static_assert(
    std::is_constructible_v<writable_data_source_handle, memory_data_buffer &>,
    "");

} // namespace maboroutu
