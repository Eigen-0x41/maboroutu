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
 *       (b) スコープ完結型パッチ - offset_patch/reserve_patch（RAII）。
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

 protected:
 private:
   using self_type = deferred_resolver;
   using patch_type = offset_patch<Bytes, Endian, Buffer>;
   using storage_type = typename patch_type::storage_type;
   using errc_type = data_source_result<void>::error_type::code_type;

   // ハンドル -> 最終オフセット（対象オブジェクトが配置された時点で確定）。
   slot_map<IndexT, std::size_t> _offsets;

   // 未解決のパッチ一覧（あるハンドルの最終オフセットが必要な箇所）。
   std::vector<std::pair<IndexT, patch_type>> _pending;

   /*--:  *IMPLIMENT_FIELD*/
 protected:
 public:
   deferred_resolver() = default;

   /**
    * @brief 論理ハンドルを発行する。対象オブジェクトはまだ存在しなくてよい。
    */
   [[nodiscard]] auto checkout(this self_type &self) -> handle_type {
      return self._offsets.checkout();
   }

   /**
    * @brief 発行済みハンドルに、実際の最終オフセットを確定する
    *        （対象オブジェクトが buf 上へ実際に配置された時点で呼ぶ）。
    * @pre handle は checkout() が返した、かつ未確定のハンドルであること。
    *      二重確定は assert で検出する（呼び出し側のバグ）。
    */
   auto resolve_handle(this self_type &self, handle_type handle,
                       std::size_t offset) -> void {
      auto const result = self._offsets.construct_at(handle, offset);
      assert(result != decltype(self._offsets)::npos &&
             "deferred_resolver: handle が二重に確定されました");
   }

   /**
    * @brief 「handle の最終オフセットで、buf の現在位置をパッチしたい」
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
    * @brief 保留中の全パッチを、対応するハンドルの最終オフセットで解決する。
    *
    * @note 未確定のハンドルが残っている場合は invalid_member_variable を
    *       返す（呼び出し側が resolve_handle() を呼び忘れた場合の検出。
    *       offset_patch 自体の「resolve()忘れ」は各パッチのデストラクタの
    *       assert が別途検出するため、ここでは「どのハンドルが未確定か」
    *       という、より上位の整合性のみを扱う）。
    * @note 失敗時は、以降処理していない保留パッチを全て abandon() する。
    *       これを行わないと、finalize() のエラーを正しく処理した呼び出し
    *       側であっても、resolver 破棄時に未解決 offset_patch の
    *       デストラクタ assert でクラッシュしてしまうため。
    */
   [[nodiscard]] auto finalize(this self_type &self)
       -> data_source_result<void> {
      for (std::size_t i = 0; i < self._pending.size(); ++i) {
         auto &[handle, patch] = self._pending[i];
         if (!self._offsets.contains(handle)) [[unlikely]] {
            self._abandon_from(i);
            return make_unexpected(errc_type::invalid_member_variable);
         }
         auto result =
             patch.resolve(static_cast<storage_type>(self._offsets[handle]));
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
