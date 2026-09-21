module;
#include <cassert>
#include <cstddef>
#include <expected>
#include <utility>
#include <vector>
export module maboroutu.binary_layout_handle;
export import maboroutu.binary_layout;
import maboroutu.core;
import maboroutu.error;
import maboroutu.data_source;
import maboroutu.data_buffer;
import maboroutu.slot_map;

namespace maboroutu {

/**
 * @brief ハンドルベースの遅延解決（tier c）。参照先オブジェクトが、
 *        参照元とは独立した文脈で後から構築される場合に使う。
 *
 * @note 分類は以下の3段階（application_spec.md 検討時）:
 *       (a) 即時解決 - offset_ref 自体が不要。
 *       (b) スコープ完結型パッチ - position_patch/reserve_patch（RAII）。
 *       (c) ハンドルベース遅延解決 - 本クラス。
 *
 * @note IndexT は mylib.slot_map が要求する形（enum class、符号なしの
 *       underlying_type）を満たす必要がある。
 * @note 現時点では、1つの deferred_resolver インスタンスは単一の
 *       Bytes/Endian の組のみを扱う（異なる幅の前方参照が同時に必要な
 *       場合は、複数の deferred_resolver を使い分ける）。
 */
export template <class IndexT, std::size_t Bytes, endian Endian,
                 data_buffer Buffer>
class deferred_resolver {
 public: /*STRUCT_FIELD*/
   using handle_type = IndexT;

 private:
   using self_type = deferred_resolver;
   using patch_type = position_patch<Bytes, Endian, Buffer>;
   using storage_type = typename patch_type::storage_type;
   using errc_type = data_source_result<void>::error_type::code_type;

   // ハンドル ->
   // 確定値（バイトオフセット・インデックスいずれも可。対象が確定した時点で決まる）。
   slot_map<IndexT, std::size_t> _positions;

   // 未解決のパッチ一覧（あるハンドルの確定値が必要な箇所）。
   std::vector<std::pair<IndexT, patch_type>> _pending;

 public:
   deferred_resolver() = default;

   /**
    * @brief 論理ハンドルを発行する。対象オブジェクトはまだ存在しなくてよい。
    */
   [[nodiscard]] auto checkout(this self_type &self) -> handle_type {
      return self._positions.checkout();
   }

   /**
    * @brief
    * 発行済みハンドルに、実際の確定値（バイトオフセットまたはインデックス）を確定する
    *        （対象オブジェクトが buf 上へ実際に配置された時点で呼ぶ）。
    * @pre handle は checkout() が返した、かつ未確定のハンドルであること。
    *      二重確定は assert で検出する（呼び出し側のバグ）。
    */
   auto resolve_handle(this self_type &self, handle_type handle,
                       std::size_t position) -> void {
      auto const result = self._positions.construct_at(handle, position);
      assert(result != decltype(self._positions)::npos &&
             "deferred_resolver: handle が二重に確定されました");
   }

   /**
    * @brief 「handle の確定値で、buf の現在位置をパッチしたい」
    *        という要求を登録する。handle はまだ resolve_handle()
    *        されていなくてもよい（finalize() まで遅延する）。
    */
   [[nodiscard]] auto defer_patch(this self_type &self, Buffer &buf,
                                  handle_type handle)
       -> data_buffer_result<void> {
      auto patch = reserve_patch<Bytes, Endian>(buf);
      if (!patch) [[unlikely]] {
         return std::unexpected(std::move(patch).error());
      }
      self._pending.emplace_back(handle, std::move(*patch));
      return {};
   }

   /**
    * @brief 保留中の全パッチを、対応するハンドルの確定値で解決する。
    *
    * @note 未確定のハンドルが残っている場合は invalid_member_variable を
    *       返す（呼び出し側が resolve_handle() を呼び忘れた場合の検出。
    *       position_patch 自体の「resolve()忘れ」は各パッチのデストラクタの
    *       assert が別途検出するため、ここでは「どのハンドルが未確定か」
    *       という、より上位の整合性のみを扱う）。
    * @note 失敗時は、以降処理していない保留パッチを全て abandon() する。
    *       これを行わないと、finalize() のエラーを正しく処理した呼び出し
    *       側であっても、resolver 破棄時に未解決 position_patch の
    *       デストラクタ assert でクラッシュしてしまうため。
    */
   [[nodiscard]] auto finalize(this self_type &self)
       -> data_source_result<void> {
      for (std::size_t i = 0; i < self._pending.size(); ++i) {
         auto &[handle, patch] = self._pending[i];
         if (!self._positions.contains(handle)) [[unlikely]] {
            self._abandon_from(i);
            return make_unexpected(errc_type::invalid_member_variable);
         }
         auto result =
             patch.resolve(static_cast<storage_type>(self._positions[handle]));
         if (!result) [[unlikely]] {
            // resolve() は失敗時も _resolved を true にするため、
            // 現在の i 自身は abandon() 不要（既に resolve() 済み扱い）。
            self._abandon_from(i + 1);
            return result;
         }
      }
      return {};
   }

 private:
   auto _abandon_from(this self_type &self, std::size_t start_index) -> void {
      for (std::size_t i = start_index; i < self._pending.size(); ++i) {
         self._pending[i].second.abandon();
      }
   }
};

} // namespace maboroutu
