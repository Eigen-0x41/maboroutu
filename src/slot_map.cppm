/**
 * @file
 * @brief 安定ハンドル（インデックス）付きフリーリスト型コンテナ
 * `basic_slot_map` の実装。
 *
 * @note
 * 本ファイルのDoxygenコメントは、library_spec_v1_16.md 4.6節（mylib.slot_map）
 *       および既存のインラインコメント・実装内容を基に付与したものであり、
 *       コードの実行時挙動そのものは変更していない（コメント追加のみ）。
 * @warning 本実装は世代（generation）カウンタを持たない。ハンドルの無効化検出が
 *          必要な場合は、値型 `T` 自体をポインタ的な型（例:
 * `std::shared_ptr`）に することを想定する（仕様書4.6節）。
 */
module;
#include <array>
#include <cassert>
#include <concepts>
#include <deque>
#include <iterator>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>
export module maboroutu.slot_map;
import :node;

namespace maboroutu {

/**
 * @brief slot_map base
 * enumを使用することでindexに数値が混入することを防いでいます
 * 要素数がIndexTの最大値-1以上となる場合の動作は未定義です。
 *
 * @details
 * 安定ハンドル（インデックス）でアクセスできる、フリーリスト方式の補助コンテナ。
 *          要素の追加（checkout+construct_at、またはinsert/emplace）・削除（erase）が
 *          いずれもO(1)で行え、追加・削除を繰り返しても既存要素へのハンドル（index_type）
 *          が変化しないという性質を持つ。
 *
 * @tparam IndexT ハンドル型。`std::is_enum_v<IndexT>` かつ
 *                `std::unsigned_integral<std::underlying_type_t<IndexT>>`
 * を満たす enum型でなければならない（requires節参照）。
 * @tparam T 格納する値の型。
 * @tparam MakeContainerT 内部コンテナ（`node_type`
 *            の配列）を選択するためのポリシー型。
 *            `template <class U> using type = ...;` の形で
 *            内部コンテナ型を提供する
 *            （例: `make_deque` は `std::deque`、
 *              `make_array<SizeV>` は `std::array<T, SizeV>`
 *              を選択する）。
 *             また、type は配列系のSTLコンテナであれば概ね使用可能。
 *
 * @warning
 * 世代（generation）カウンタを持たない実装であるため、erase()後に再利用された
 *          スロットへ古いハンドルでアクセスした場合の無効化検出はできない
 *          （仕様書4.6節「位置づけ（重要な設計上の注記）」参照）。
 * @note `get_if` は `std::variant::get_if` の前例に倣い自由関数として提供する
 *       （メンバ関数 `extension_store::get_if`
 * とは異なる設計判断。仕様書1.5節参照）。
 */
// [[basic_slot_map]]
export template <class IndexT, class T, class MakeContainerT>
   requires std::is_enum_v<IndexT> &&
            std::unsigned_integral<std::underlying_type_t<IndexT>>
class basic_slot_map {
 public: /*STRUCT_FIELD*/
   //! @brief 安定ハンドル（要素へアクセスするためのキー）の型。テンプレート引数
   //! IndexT。
   using index_type = IndexT;
   //! @brief 格納する値の型。テンプレート引数 T。
   using value_type = T;
   //! @brief size()/free_size() が返すサイズ表現型。
   using size_type = std::size_t;

   //! @brief
   //! 「無効なハンドル」または「フリーリスト／構築済みリストの終端」を表す番兵値。
   static constexpr index_type npos = static_cast<index_type>(-1);

 protected:
 private:
   //! @brief 自身の型（deducing this 用の略記）。
   using self_type = basic_slot_map;

   //! @brief 内部の帳簿管理（prev/nextリンク・番兵値比較）専用のインデックス型
   //!        （=
   //!        `std::underlying_type_t<IndexT>`）。`size_type`（`size_t`）とは独立しており、
   //!        `IndexT` の基底型の幅・符号にかかわらず番兵値比較の整合性を保つ。
   using iindex_type = std::underlying_type_t<index_type>;
   //! @brief 要素の構築状態・prev/nextリンク情報を保持する内部ノード型
   //!        （モジュールパーティション `:node` で定義）。
   using node_type = slot_map_node<basic_slot_map, T, iindex_type>;
   //! @brief `MakeContainerT` により選択される、`node_type`
   //! を格納する実コンテナ型。
   using container_type = MakeContainerT::template type<node_type>;

   /**
    * @brief 構築済み要素のみを走査する双方向イテレータの実装本体。
    * @tparam IsConst true の場合 const版（const_iterator）として振る舞う。
    */
   template <bool IsConst>
   // [[basic_iterator]]
   class basic_iterator {
    public: /*STRUCT_FIELD*/
      using difference_type = std::ptrdiff_t;
      using value_type = typename std::pair<
          index_type const,
          typename std::conditional_t<IsConst, value_type const, value_type> &>;
      using iterator_concept = std::bidirectional_iterator_tag;

      //! @brief `operator->()` が返す、`value_type`（`std::pair<index_type
      //! const, T&>`）を
      //!        保持するだけの一時プロキシ型。
      class pointer {
       private:
         value_type _value;

       public:
         pointer(value_type value) : _value(value) {}
         auto operator*() -> value_type & { return _value; }
         auto operator->() -> value_type * { return &_value; }
      };

    protected:
    private:
      friend basic_iterator<!IsConst>;

      using self_type = basic_iterator;
      using depend_type =
          typename std::conditional_t<IsConst, basic_slot_map const,
                                      basic_slot_map>;

      friend basic_slot_map;

      depend_type *_data;
      iindex_type _idx_s;

      /*--:  *IMPLIMENT_FIELD*/
      explicit basic_iterator(depend_type *data, iindex_type idx_s)
          : _data(data), _idx_s(idx_s) {}

    protected:
    public:
      basic_iterator() = default;
      basic_iterator(basic_iterator const &) = default;
      basic_iterator(basic_iterator &&) = default;
      basic_iterator(basic_iterator<false> const &ite)
         requires IsConst
          : _data(ite._data), _idx_s(ite._idx_s) {}
      ~basic_iterator() = default;

      //! @brief 前置インクリメント。次に構築済みの要素（なければ
      //! end()）へ進む。
      friend constexpr auto operator++(self_type &self) -> self_type & {
         self._idx_s =
             (static_cast<iindex_type>(depend_type::npos) != self._idx_s)
                 ? self._data->_container[self._idx_s].next()
                 : self._data->_next_constructed;
         return self;
      }
      //! @brief 後置インクリメント。
      friend constexpr auto operator++(self_type &self, int) -> self_type {
         self_type ret_value = self;
         ++self;
         return ret_value;
      }

      //! @brief 前置デクリメント。1つ前に構築済みの要素へ戻る。
      friend constexpr auto operator--(self_type &self) -> self_type & {
         self._idx_s =
             (static_cast<iindex_type>(depend_type::npos) != self._idx_s)
                 ? self._data->_container[self._idx_s].prev()
                 : self._data->_rnext_constructed;
         return self;
      }
      //! @brief 後置デクリメント。
      friend constexpr auto operator--(self_type &self, int) -> self_type {
         self_type ret_value = self;
         --self;
         return ret_value;
      }

      //! @brief 間接参照。`(index_type const, value_type&)` のペアを返す。
      //! @pre 現在位置が構築済みの要素を指していること（未構築の場合 assert）。
      friend constexpr auto operator*(self_type const &self) -> value_type {
         auto &data = self._data->_container[self._idx_s];
         assert(data.has_value());
         return std::make_pair(static_cast<index_type>(self._idx_s),
                               std::ref(data.value()));
      }

      //! @brief アロー演算子。`*self` を保持する一時 `pointer` プロキシを返す。
      constexpr auto operator->(this self_type const &self) -> pointer {
         return {*self};
      }

      //! @brief
      //! 等価比較。参照元コンテナと現在位置（インデックス）がともに一致する場合に
      //! true。
      template <bool RConst>
      friend constexpr auto operator==(self_type const &lhs,
                                       basic_iterator<RConst> const &rhs)
          -> bool {
         return (lhs._data == rhs._data) && (lhs._idx_s == rhs._idx_s);
      }

      auto operator=(basic_iterator const &rhs) -> basic_iterator & = default;
      auto operator=(basic_iterator &&rhs) -> basic_iterator & = default;
      auto operator=(basic_iterator<false> const &ite) -> basic_iterator &
         requires IsConst
      {
         _data = ite._data;
         _idx_s = ite._idx_s;
         return *this;
      }
   };

   //! @brief ノード実体を保持する内部コンテナ本体。
   container_type _container{};
   //! @brief 構築済みリストの先頭（begin()）を指す iindex_type。空の場合 npos。
   iindex_type _next_constructed = static_cast<iindex_type>(self_type::npos);
   //! @brief 構築済みリストの末尾（rbegin()相当、逆順走査の起点）を指す
   //! iindex_type。
   iindex_type _rnext_constructed = static_cast<iindex_type>(self_type::npos);
   //! @brief フリーリスト（未構築スロットの片方向連結リスト）の先頭を指す
   //! iindex_type。
   iindex_type _next_destroyed = static_cast<iindex_type>(self_type::npos);
   //! @brief 現在構築済みの要素数。
   size_type _size = 0;
   //! @brief 現在フリーリストにある（未構築の）スロット数。
   size_type _free_size = 0;

   /*--:  *IMPLIMENT_FIELD*/
   //! @brief `key` が番兵値 npos と等しいかどうかを判定する。
   static constexpr auto is_npos(iindex_type key) noexcept -> bool {
      return key == static_cast<iindex_type>(self_type::npos);
   }

 protected:
 public:
   //! @brief 構築済み要素を走査する、書き込み可能な双方向イテレータ。
   using iterator = basic_iterator<false>;
   //! @brief 構築済み要素を走査する、読み取り専用の双方向イテレータ。
   using const_iterator = basic_iterator<true>;

   /**
    * @brief デフォルトコンストラクタ。
    * @details 内部コンテナが固定長かつ非ゼロサイズ（`inplace_slot_map`
    * の典型）の場合、
    *          全スロットを未構築状態のままフリーリストへ連結する。先頭要素の
    * prev()、 末尾要素の next() は、いずれも
    * npos（フリーリストの終端）として明示的に 設定される。
    * @note （v1.16で修正）以前の実装では末尾要素の next()
    * にコンテナサイズという
    *       範囲外の値が入ってしまうバグがあった。全スロット使用後に更に
    * checkout() を 呼び出すと `std::array`
    * への境界外アクセス（未定義動作）が発生する不具合が あったため、末尾要素の
    * next() を npos へ設定するよう修正済み（library_spec_v1_16.md
    *       4.6節・変更履歴v1.16参照）。
    * @note 内部コンテナが可変長（例: `slot_map` の
    * `std::deque`）で初期サイズ0の場合は
    *       このフリーリスト初期化ループは実行されない（要素は
    * checkout()/insert() 等の コンテナ拡張パスで都度追加される）。
    */
   basic_slot_map() {
      // 最適化されることを望む。
      if (_container.size() != 0) {
         assert(_container.size() < std::numeric_limits<iindex_type>::max());
         _free_size = _container.size();
#pragma unroll 2
         for (auto &&[idx, node] :
              std::views::zip(std::views::iota(0), _container)) {
            node.prev() = iindex_type(idx) - 1;
            node.next() = iindex_type(idx) + 1;
         }

         _container.front().prev() = static_cast<iindex_type>(self_type::npos);
         _container.back().next() = static_cast<iindex_type>(self_type::npos);
         _next_destroyed = 0;
      }
   }
   //! @brief
   //! コピーコンストラクタ（既定の実装。内部コンテナ・帳簿管理情報を丸ごと複製する）。
   basic_slot_map(basic_slot_map const &) = default;
   //! @brief ムーブコンストラクタ（既定の実装）。
   basic_slot_map(basic_slot_map &&) = default;
   //! @brief
   //! デストラクタ（既定の実装。要素破棄は内部コンテナ・ノードの解体に委ねる）。
   ~basic_slot_map() = default;

   /**
    * @brief `index` が現在構築済みの要素を指しているかどうかを判定する。
    * @param index 判定対象のハンドル。
    * @return `index` が npos
    * でも範囲外でもなく、かつ対応するスロットが構築済みの場合 true。
    */
   [[nodiscard]] auto contains(this self_type const &self,
                               index_type const index) noexcept -> bool {
      auto idx = static_cast<iindex_type>(index);
      if (self_type::is_npos(idx)) [[unlikely]] {
         return false;
      }
      if (idx >= self._container.size()) [[unlikely]] {
         return false;
      }
      return bool(self._container[idx]);
   }

   /**
    * @brief `key` に対応する値への参照を、範囲・存在チェック付きで取得する。
    * @tparam Self 呼び出し時に推論される、
    *         cv修飾を含む自身の型（deducing this）。
    *         const修飾されている場合は value_type const を返す。
    * @param key 取得対象のハンドル。
    * @return 値への参照。
    * @throw std::out_of_range `contains(key)` が false
    * の場合（未構築・npos・範囲外）。
    */
   template <class Self>
   auto at(this Self &self, index_type const key)
       -> std::conditional_t<std::is_const_v<Self>, value_type const,
                             value_type> & {
      if (!self.contains(key)) [[unlikely]] {
         throw std::out_of_range("not contain the specified key");
      }
      return self._container[static_cast<iindex_type>(key)].value();
   }

   /**
    * @brief `key` に対応する値への参照を、チェックなしで取得する。
    * @tparam Self 呼び出し時に推論される、
    *         cv修飾を含む自身の型（deducing this）。
    *         const修飾されている場合は value_type const を返す。
    * @param key 取得対象のハンドル。
    * @return 値への参照。
    * @pre `contains(key)` が true
    * であること（呼び出し側の責任。デバッグビルドでは assert）。
    */
   template <class Self>
   auto operator[](this Self &self, index_type const key) noexcept
       -> std::conditional_t<std::is_const_v<Self>, value_type const,
                             value_type> & {
      assert(self.contains(key));
      return self._container[static_cast<iindex_type>(key)].value();
   }

   //! @brief 現在構築済みの要素数を返す。
   [[nodiscard]] auto size(this self_type const &self) noexcept -> size_type {
      return self._size;
   }
   //! @brief 現在フリーリストにある（未構築の）スロット数を返す。
   [[nodiscard]] auto free_size(this self_type const &self) noexcept
       -> size_type {
      return self._free_size;
   }

   /**
    * @brief
    * 未構築のスロットを1つ予約し、そのハンドルを返す（値はまだ構築されない）。
    * @details
    * フリーリストに空きがあればそこから再利用し、なければ内部コンテナを
    *          拡張する（`push_back` を持つ可変長コンテナの場合のみ）。
    * @return 予約したスロットを指すハンドル。
    * @throw std::out_of_range 内部コンテナが
    *           `push_back()`を持たず（固定長コンテナ等）、
    *           かつフリーリストが空でこれ以上拡張できない場合。
    * @throw std::out_of_range インデックス空間がIndexTの表現範囲を超えた場合。
    * @note 返されたハンドルは construct_at() で値を構築するか、cancel() で
    *       フリーリストへ戻すまで、構築済みリストには一切リンクされない。
    */
   [[nodiscard]] auto checkout(this self_type &self) -> index_type {
      if (!self_type::is_npos(self._next_destroyed)) {
         iindex_type construct_target = self._next_destroyed;
         node_type &target = self._container[construct_target];

         self._next_destroyed = target.next();
         target.prev() = static_cast<iindex_type>(self_type::npos);
         target.next() = static_cast<iindex_type>(self_type::npos);

         --self._free_size;
         return static_cast<index_type>(construct_target);
      }

      if constexpr (requires() { self._container.push_back(node_type()); }) {
         iindex_type construct_target(self._container.size());
         if (construct_target == static_cast<iindex_type>(npos)) [[unlikely]] {
            throw std::out_of_range("slot_map index exhausted");
         }
         // NOTE:
         // node_typeはデフォルトnode_type()がprev()とnext()の両方をnposにする。
         self._container.push_back(node_type());

         return static_cast<index_type>(construct_target);
      }
      throw std::out_of_range("Continer is not have push_back().");
   }

   /**
    * @brief checkout() で予約した未構築スロットを予約前の状態へ戻す。
    * @param key checkout() が返した未構築スロットのハンドル。
    * @pre `key` は、このコンテナに対する直前の `checkout()` によって
    *      取得されたハンドルであり、かつ `construct_at()` または
    *      `cancel()` がまだ呼び出されていないこと。
    * @pre `key` は `npos` ではなく、内部コンテナの範囲内であること。
    * @warning 上記の事前条件が満たされない場合の動作は未定義。
    * @post スロットはフリーリストへ戻され、`free_size()` が1増加する。
    * @post 構築済み要素数 `size()` は変化しない。
    * @note `cancel()` は `checkout()` と対になる操作であり、
    *       checkout 済みスロットを再び `checkout()` 可能な状態へ戻す。
    */
   auto cancel(this self_type &self, index_type key) -> void {
      node_type &target = self._container[static_cast<iindex_type>(key)];
      assert(!target.has_value());

      target.next() = self._next_destroyed;
      self._next_destroyed = static_cast<iindex_type>(key);
      ++self._free_size;
   }

   /**
    * @brief checkout() で予約したスロットに値を構築する。
    * @param key checkout() が返した未構築スロットのハンドル。
    * @param value 構築する値。
    * @return 構築に成功した場合は `key`、対象スロットが既に構築済みの場合は
    *         `npos`。
    * @pre `key` は `checkout()`によって取得された
    *      有効な未構築スロットであること。
    * @pre `key` は `npos` ではなく、内部コンテナの範囲内であること。
    * @warning 上記の事前条件が満たされない場合の動作は未定義。
    * @post 成功した場合、対象スロットは構築済みリストの先頭に追加される。
    * @post 成功した場合、`size()` が1増加する。
    * @exception value_type のコピーコンストラクタが例外を送出した場合、
    *            コンテナの状態は変更されない。
    */
   auto construct_at(this self_type &self, index_type key,
                     value_type const &value) -> index_type {
      assert(!self_type::is_npos(static_cast<iindex_type>(key)));
      assert(static_cast<iindex_type>(key) < self._container.size());

      node_type &target = self._container[static_cast<iindex_type>(key)];

      if (target.has_value()) [[unlikely]] {
         return self_type::npos;
      }

      target.construct(value);

      // construct linking
      target.prev() = static_cast<iindex_type>(self_type::npos);
      if (self.size() != 0) [[likely]] {
         auto &next = self._container[self._next_constructed];
         assert(self_type::is_npos(next.prev()));
         next.prev() = static_cast<iindex_type>(key);
      } else {
         self._rnext_constructed = static_cast<iindex_type>(key);
      }

      target.next() = self._next_constructed;
      self._next_constructed = static_cast<iindex_type>(key);

      ++self._size;
      return key;
   }
   /**
    * @brief checkout() で予約したスロットに値を in-place 構築する。
    * @tparam ArgsT `value_type` のコンストラクタへ渡す引数の型。
    * @param key checkout() が返した未構築スロットのハンドル。
    * @param args `value_type` のコンストラクタへ転送する引数。
    * @return 構築に成功した場合は `key`、対象スロットが既に構築済みの場合は
    *         `npos`。
    * @pre `key` は `checkout()`によって取得された
    *      有効な未構築スロットであること。
    * @pre `key` は `npos` ではなく、内部コンテナの範囲内であること。
    * @warning 上記の事前条件が満たされない場合の動作は未定義。
    * @post 成功した場合、対象スロットは構築済みリストの先頭に追加される。
    * @post 成功した場合、`size()` が1増加する。
    * @exception `value_type` のコンストラクタが例外を送出した場合、
    *            コンテナの状態は変更されない。
    */
   template <class... ArgsT>
   auto construct_at(this self_type &self, index_type key, ArgsT &&...args)
       -> index_type {
      assert(!self_type::is_npos(static_cast<iindex_type>(key)));
      assert(static_cast<iindex_type>(key) < self._container.size());

      node_type &target = self._container[static_cast<iindex_type>(key)];

      if (target.has_value()) [[unlikely]] {
         return self_type::npos;
      }

      target.construct(std::forward<ArgsT>(args)...);

      // construct linking
      target.prev() = static_cast<iindex_type>(self_type::npos);
      if (self.size() != 0) [[likely]] {
         auto &next = self._container[self._next_constructed];
         assert(self_type::is_npos(next.prev()));
         next.prev() = static_cast<iindex_type>(key);
      } else {
         self._rnext_constructed = static_cast<iindex_type>(key);
      }

      target.next() = self._next_constructed;
      self._next_constructed = static_cast<iindex_type>(key);

      ++self._size;
      return key;
   }

   /**
    * @brief 値をコピーして新しい要素を構築する。
    * @param value 構築する値。
    * @return 構築された要素のハンドル。
    * @throw std::out_of_range 新しいスロットを確保できない場合。
    * @exception `value_type` のコピーコンストラクタが例外を送出した場合、
    *            その例外を伝播する。コンテナの状態は変更されない。
    * @post 成功した場合、要素は構築済みリストの先頭に追加される。
    * @post 成功した場合、`size()` が1増加する。
    * @note フリーリストに空きがある場合、そのスロットを再利用する。
    *       空きがない場合、内部コンテナが `push_back()` を提供していれば
    *       新しいスロットを追加する。
    */
   auto insert(this self_type &self, value_type const &value) -> index_type {
      if (!self_type::is_npos(self._next_destroyed)) {
         iindex_type construct_target = self._next_destroyed;
         node_type &target = self._container[construct_target];

         target.construct(value);
         self._next_destroyed = target.next();

         // construct linking
         target.prev() = static_cast<iindex_type>(self_type::npos);
         if (self.size() != 0) [[likely]] {
            auto &next = self._container[self._next_constructed];
            assert(self_type::is_npos(next.prev()));
            next.prev() = construct_target;
         } else {
            self._rnext_constructed = construct_target;
         }

         target.next() = self._next_constructed;
         self._next_constructed = construct_target;

         --self._free_size;
         ++self._size;
         return static_cast<index_type>(construct_target);
      }

      if constexpr (requires() {
                       self._container.push_back(node_type(
                           {
                               .prev =
                                   static_cast<iindex_type>(self_type::npos),
                               .next = self._next_constructed,
                           },
                           value));
                    }) {
         iindex_type construct_target(self._container.size());
         if (construct_target == static_cast<iindex_type>(npos)) [[unlikely]] {
            throw std::out_of_range("slot_map index exhausted");
         }
         self._container.push_back(node_type(
             {
                 .prev = static_cast<iindex_type>(self_type::npos),
                 .next = self._next_constructed,
             },
             value));
         if (self.size() != 0) [[likely]] {
            auto &next = self._container[self._next_constructed];
            assert(self_type::is_npos(next.prev()));
            next.prev() = construct_target;
         } else {
            self._rnext_constructed = construct_target;
         }
         self._next_constructed = construct_target;
         ++self._size;
         return static_cast<index_type>(construct_target);
      }
      throw std::out_of_range("Continer is not have push_back().");
   }

   /**
    * @brief 値を in-place 構築して新しい要素を追加する。
    * @tparam ArgsT `value_type` のコンストラクタへ渡す引数の型。
    * @param args `value_type` のコンストラクタへ転送する引数。
    * @return 構築された要素のハンドル。
    * @throw std::out_of_range 新しいスロットを確保できない場合。
    * @exception `value_type` のコンストラクタが例外を送出した場合、
    *            その例外を伝播する。コンテナの状態は変更されない。
    * @post 成功した場合、要素は構築済みリストの先頭に追加される。
    * @post 成功した場合、`size()` が1増加する。
    * @note フリーリストに空きがある場合、そのスロットを再利用する。
    *       空きがない場合、内部コンテナが `emplace_back()` を提供していれば
    *       新しいスロットを追加する。
    */
   template <class... ArgsT>
   auto emplace(this self_type &self, ArgsT &&...args) -> index_type {
      if (!self_type::is_npos(self._next_destroyed)) {
         iindex_type construct_target = self._next_destroyed;
         node_type &target = self._container[construct_target];

         target.construct(std::forward<ArgsT>(args)...);
         self._next_destroyed = target.next();

         // construct linking
         target.prev() = static_cast<iindex_type>(self_type::npos);
         if (self.size() != 0) [[likely]] {
            auto &next = self._container[self._next_constructed];
            assert(self_type::is_npos(next.prev()));
            next.prev() = construct_target;
         } else {
            self._rnext_constructed = construct_target;
         }

         target.next() = self._next_constructed;
         self._next_constructed = construct_target;

         --self._free_size;
         ++self._size;
         return static_cast<index_type>(construct_target);
      }

      if constexpr (requires() {
                       self._container.emplace_back(
                           typename node_type::link{
                               .prev =
                                   static_cast<iindex_type>(self_type::npos),
                               .next = self._next_constructed,
                           },
                           std::forward<ArgsT>(args)...);
                    }) {
         iindex_type construct_target(self._container.size());
         if (construct_target == static_cast<iindex_type>(npos)) [[unlikely]] {
            throw std::out_of_range("slot_map index exhausted");
         }
         self._container.emplace_back(
             typename node_type::link{
                 .prev = static_cast<iindex_type>(self_type::npos),
                 .next = self._next_constructed,
             },
             std::forward<ArgsT>(args)...);
         if (self.size() != 0) [[likely]] {
            auto &next = self._container[self._next_constructed];
            assert(self_type::is_npos(next.prev()));
            next.prev() = construct_target;
         } else {
            self._rnext_constructed = construct_target;
         }
         self._next_constructed = construct_target;
         ++self._size;
         return static_cast<index_type>(construct_target);
      }
      throw std::out_of_range("Continer is not have emplace_back().");
   }

   /**
    * @brief `key` が指す要素を破棄し、対応するスロットをフリーリストへ戻す。
    * @param key 破棄対象のハンドル。
    * @throw std::out_of_range `contains(key)` が false の場合。
    * @note 破棄後、同一スロットは checkout()/insert()/emplace()
    * により再利用され得る。
    * 例外が送出された場合、要素は削除されない。
    */
   void erase(this self_type &self, index_type const key) {
      if (!self.contains(key)) [[unlikely]] {
         throw std::out_of_range("Key is not contains.");
      }

      node_type &target = self._container[static_cast<iindex_type>(key)];

      target.destroy();
      ++self._free_size;
      --self._size;

      if (!self_type::is_npos(target.next())) [[likely]] {
         auto &next = self._container[target.next()];
         next.prev() = target.prev();
         if (!self_type::is_npos(target.prev())) [[likely]] {
            auto &prev = self._container[target.prev()];
            prev.next() = target.next();
         } else {
            self._next_constructed = target.next();
         }
      } else {
         self._rnext_constructed = target.prev();
         if (!self_type::is_npos(target.prev())) [[likely]] {
            auto &prev = self._container[target.prev()];
            prev.next() = static_cast<iindex_type>(self_type::npos);
         } else {
            self._next_constructed = static_cast<iindex_type>(self_type::npos);
         }
      }

      target.next() = self._next_destroyed;
      self._next_destroyed = static_cast<iindex_type>(key);
#if !defined(NDEBUG)
      // MEMO: この値は現状使用されていません。
      //       assert()にて、初期化されていることを確認するために使用しています。
      target.prev() = static_cast<iindex_type>(self_type::npos);
#endif /*!defined(NDEBUG) */
   }

   // 未検証のためコメントアウト
   // @brief（未実装/未検証）末尾の未使用スロットをコンテナから切り詰める。
   // @warning
   // フリーリストが片方向連結リストであるため、erase()で戻されたノードの
   //          prev()は意味を持たない（本体上部コメント参照）。この性質と
   //          shrink() の
   //          実装が整合するかは再検証・再設計が必要であり、使用は非推奨（8章参照）。
   //    constexpr void shrink(this self_type &self) {
   // #pragma unroll 2
   //       for (auto index = iindex_type(self._container.size()); index != 0;)
   //       {
   //          --index;
   //          if (self._container[index]) [[unlikely]] {
   //             break;
   //          }
   //
   //          auto &target = self._container[index];
   //          if (index == self._next_constructed) [[unlikely]] {
   //             self._next_constructed = target.next();
   //          }
   //          if (target.prev() != static_cast<iindex_type>(self_type::npos))
   //              [[likely]] {
   //             self._container[target.prev()].next() = target.next();
   //          }
   //          if (target.next() != static_cast<iindex_type>(self_type::npos))
   //              [[likely]] {
   //             self._container[target.next()].prev() = target.prev();
   //          }
   //
   //          self._container.pop_back();
   //          self._free_size--;
   //       }
   //       self._container.shrink_to_fit();
   //    }

   /**
    * @brief 構築済み要素の先頭を指すイテレータを返す。
    * @tparam Self 呼び出し時に推論される、
    *         cv修飾を含む自身の型（deducing this）。
    *         const修飾されている場合は const_iterator を返す。
    * @return const版なら const_iterator、非const版なら iterator。
    */
   template <class Self>
   constexpr auto begin(this Self &self) noexcept
       -> std::conditional_t<std::is_const_v<Self>, const_iterator, iterator> {
      if constexpr (std::is_const_v<Self>) {
         return const_iterator(&self, self._next_constructed);
      } else {
         return iterator(&self, self._next_constructed);
      }
   }
   //! @brief 常に const_iterator を返す begin()。
   constexpr auto cbegin(this self_type const &self) noexcept
       -> const_iterator {
      return const_iterator(&self, self._next_constructed);
   }

   /**
    * @brief 構築済み要素の終端（番兵、npos）を指すイテレータを返す。
    * @tparam Self 呼び出し時に推論される、
    *         cv修飾を含む自身の型（deducing this）。
    *         const修飾されている場合は const_iterator を返す。
    * @return const版なら const_iterator、非const版なら iterator。
    */
   template <class Self>
   constexpr auto end(this Self &self) noexcept
       -> std::conditional_t<std::is_const_v<Self>, const_iterator, iterator> {
      if constexpr (std::is_const_v<Self>) {
         return const_iterator(&self,
                               static_cast<iindex_type>(self_type::npos));
      } else {
         return iterator(&self, static_cast<iindex_type>(self_type::npos));
      }
   }
   //! @brief 常に const_iterator を返す end()。
   constexpr auto cend(this self_type const &self) noexcept -> const_iterator {
      return const_iterator(&self, static_cast<iindex_type>(self_type::npos));
   }

   //! @brief コピー代入演算子（既定の実装）。
   auto operator=(basic_slot_map const &rhs) -> basic_slot_map & = default;
   //! @brief ムーブ代入演算子（既定の実装）。
   auto operator=(basic_slot_map &&rhs) -> basic_slot_map & = default;
};

/**
 * @brief `index`
 * が指す要素へのポインタを、存在チェック付きで取得する（`std::get_if` 準拠）。
 * @param slot_map 検索対象の basic_slot_map。
 * @param index 検索対象のハンドル。
 * @return 要素が存在すればそのポインタ、存在しなければ nullptr。
 */
export template <class IndexT, class T, class MakeContainerT>
auto get_if(
    basic_slot_map<IndexT, T, MakeContainerT> &slot_map,
    typename basic_slot_map<IndexT, T, MakeContainerT>::index_type index)
    -> basic_slot_map<IndexT, T, MakeContainerT>::value_type * {
   if (!slot_map.contains(index)) [[unlikely]] {
      return nullptr;
   }
   return &slot_map[index];
}

/**
 * @brief `index`
 * が指す要素へのポインタを、存在チェック付きで取得する（`std::get_if` 準拠）。
 * @param slot_map 検索対象の basic_slot_map。
 * @param index 検索対象のハンドル。
 * @return 要素が存在すればそのポインタ、存在しなければ nullptr。
 */
export template <class IndexT, class T, class MakeContainerT>
auto get_if(
    basic_slot_map<IndexT, T, MakeContainerT> const &slot_map,
    typename basic_slot_map<IndexT, T, MakeContainerT>::index_type index)
    -> basic_slot_map<IndexT, T, MakeContainerT>::value_type const * {
   if (!slot_map.contains(index)) [[unlikely]] {
      return nullptr;
   }
   return &slot_map[index];
}

//! @brief `basic_slot_map` の `MakeContainerT` 引数用ポリシー。内部コンテナに
//!        `std::deque<node_type>`（可変長）を選択する。既定の `slot_map`
//!        エイリアスで使用。
struct make_deque {
   template <class T> using type = typename std::deque<T>;
};
//! @brief `basic_slot_map` の `MakeContainerT` 引数用ポリシー。内部コンテナに
//!        `std::array<node_type,
//!        SizeV>`（固定長）を選択する。`inplace_slot_map` エイリアスで使用。
//! @tparam SizeV 固定長コンテナの要素数。
template <size_t SizeV> struct make_array {
   template <class T> using type = typename std::array<T, SizeV>;
};

//! @brief 内部コンテナに `std::deque` を用いる、既定の可変長版
//! `basic_slot_map`。
export template <class IndexT, class T>
using slot_map = basic_slot_map<IndexT, T, make_deque>;
//! @brief 内部コンテナに `std::array<T, SizeV>` を用いる、固定長版
//! `basic_slot_map`。
//! @tparam SizeV 固定長コンテナの要素数（＝格納可能な最大要素数）。
export template <class IndexT, class T, size_t SizeV>
using inplace_slot_map = basic_slot_map<IndexT, T, make_array<SizeV>>;

} // namespace maboroutu
