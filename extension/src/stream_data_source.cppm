module;

#if (__STDC_HOSTED__ != 0) ||                                                  \
    defined(MABOROUTU_STREAM_DATA_SOURCE_ALLOW_FREESTANDING)
#include <cassert>
#include <concepts>
#include <cstddef>
#include <expected>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <limits>
#include <memory>
#include <span>
#include <type_traits>
#endif

export module maboroutu.stream_data_source;

#if (__STDC_HOSTED__ == 0) &&                                                  \
    !defined(MABOROUTU_STREAM_DATA_SOURCE_ALLOW_FREESTANDING)
static_assert(
    false,
    "maboroutu.stream_data_source depends on <istream>/<iostream>, which "
    "are NOT part of any standardized freestanding library subset as of "
    "this writing (no __cpp_lib_freestanding_iostream feature test macro "
    "exists). This module is therefore disabled by default under "
    "freestanding implementations, unlike maboroutu.error's guard (v1.28), "
    "which checks feature-test macros that DO exist for <expected>/"
    "<variant>/<memory>. "
    "If your freestanding implementation happens to provide a conforming "
    "<iostream> (a non-standard extension), define "
    "MABOROUTU_STREAM_DATA_SOURCE_ALLOW_FREESTANDING before importing this "
    "module to explicitly acknowledge that this is unverified by the "
    "standard, and proceed at your own risk. Actual runtime conformance "
    "is the responsibility of the library user (see library_spec.md 3章 "
    "freestanding guard policy).");
#else

export import maboroutu.data_source;

namespace maboroutu {

// [[stream_data_source]]
export template <std::derived_from<std::istream> Stream>
class stream_data_source {
 public: /*STRUCT_FIELD*/
   using value_type = Stream;
   template <class T> using result_type = data_source_result<T>;

 protected:
   value_type data;

 private:
   using self_type = stream_data_source;
   using errc_type = result_type<void>::error_type::code_type;

   /*--:  *IMPLIMENT_FIELD*/
 protected:
   /**
    * @brief 入力ストリームの位置を移動します。
    *
    * この関数は、指定された位置に入力ストリームを移動します。
    *
    * @param self 現在のオブジェクト。
    * @param pos 新しい位置。
    * @return result_type<void> 成功または失敗を示す結果オブジェクト。
    *
    * @note 移動操作が失敗した場合、`operation_failure`
    * のエラーコードが返されます。
    */
   template <class Self>
   [[nodiscard]] auto seekg(this Self &self, std::size_t pos)
       -> result_type<void> {
      // NOTE: pos_typeはclassで定義されているため
      //       assertは諦める。
      self.data.seekg(pos);
      if (self.data.fail()) [[unlikely]] {
         return make_unexpected(errc_type::operation_failure);
      }
      return {};
   }
   /**
    * @brief 移動ファイルポインタの位置
    *
    * @details
    * この関数は、ファイルポインタを指定された位置に移動します。
    *
    * @tparam Self テンプレートパラメータ
    * @param self 移動操作を実行するオブジェクト
    * @param pos 移動先の位置
    * @param stpos ストリーム位置の基準
    * @return result_type<void> 結果を表すresultオブジェクト
    *
    * @note
    * @li posが無効な場合、operation_failureエラーコードが返されます。
    * @li
    * stposがstd::ios_base::beg、std::ios_base::cur、std::ios_base::endのいずれかでない場合、operation_failureエラーコードが返されます。
    * @li 自分自身の右値参照として使用されることが期待されます。
    *
    * @see std::istream::seekg
    */
   template <class Self>
   [[nodiscard]] auto seekg(this Self &self, std::size_t pos,
                            std::ios_base::seekdir dir) -> result_type<void> {
      assert(pos <= std::numeric_limits<typename value_type::off_type>::max());
      self.data.seekg(pos, dir);
      if (self.data.fail()) [[unlikely]] {
         return make_unexpected(errc_type::operation_failure);
      }
      return {};
   }

 public:
   stream_data_source() = delete;
   stream_data_source(stream_data_source const &) = delete;
   stream_data_source(stream_data_source &&rhs) = default;
   template <class... Args>
      requires(!std::same_as<std::remove_cvref_t<Args>, stream_data_source> ||
               ...)
   explicit stream_data_source(Args &&...args)
       : data(std::forward<Args>(args)...) {}
   ~stream_data_source() = default;

   template <class Self>
   [[nodiscard]] auto read(this Self &self, region reg)
       -> result_type<byte_array> {
      assert(reg.size <= std::numeric_limits<std::streamsize>::max());
      if (!self.data) [[unlikely]] {
         if (self.data.bad()) [[unlikely]] {
            return make_unexpected(errc_type::invalid_member_variable);
         }
         self.data.clear();
      }
      if (auto result = self.seekg(reg.offset); !result) {
         return std::unexpected{std::move(result).error()};
      }
      byte_array ret_value{
          .value = std::make_unique<decltype(ret_value)::value_type>(reg.size),
          .size = reg.size,
      };
      self.data.read(reinterpret_cast<char *>(ret_value.value.get()),
                     ret_value.size);
      // NOTE: 指定サイズを読み込む前にEOFに達した場合はfailとなる。
      if (self.data.fail()) [[unlikely]] {
         // NOTE: 仕様によるとbadbitになることは無いらしい。
         //       ただし、fail()がbadbitも検出するためその回避。
         if (self.data.bad()) [[unlikely]] {
            return make_unexpected(errc_type::invalid_member_variable);
         }
         return make_unexpected(
             (static_cast<std::size_t>(self.data.gcount()) != reg.size)
                 ? errc_type::out_of_range
                 : errc_type::operation_failure);
      }
      return ret_value;
   }

   /**
    * @brief streamのサイズを取得する。
    * 格納されているバイナリ値を変更しないことのみ保証する。
    *
    * この関数の場合、
    * サイズを取得した際後のポジションは先頭となる。
    *
    * @tparam Self [TODO:tparam]
    * @return [TODO:return]
    */
   template <class Self>
   [[nodiscard]] auto size(this Self &self) -> result_type<std::size_t> {
      if (!self.data) [[unlikely]] {
         if (self.data.bad()) [[unlikely]] {
            return make_unexpected(errc_type::invalid_member_variable);
         }
         self.data.clear();
      }
      if (auto result = self.seekg(0, std::ios_base::end); !result) {
         return std::unexpected{std::move(result).error()};
      }
      auto const endpos = self.data.tellg();
      if (auto result = self.seekg(0); !result) {
         return std::unexpected{std::move(result).error()};
      }
      return endpos - self.data.tellg();
   }

   auto operator=(stream_data_source const &rhs)
       -> stream_data_source & = delete;
   auto operator=(stream_data_source &&rhs) -> stream_data_source & = default;
};

// [[stream_writable_data_source]]
export template <std::derived_from<std::iostream> Stream>
class stream_writable_data_source : public stream_data_source<Stream> {
 public: /*STRUCT_FIELD*/
   using value_type = Stream;

 protected:
   // value_type data;

 private:
   using self_type = stream_writable_data_source;
   template <class T>
   using result_type = stream_data_source<Stream>::template result_type<T>;
   using errc_type = result_type<void>::error_type::code_type;

   /*--:  *IMPLIMENT_FIELD*/
 protected:
   template <class Self>
   [[nodiscard]] auto seekp(this Self &self, std::size_t pos)
       -> result_type<void> {
      // NOTE: pos_typeはclassで定義されているため
      //       assertは諦める。
      self.data.seekp(pos);
      if (self.data.fail()) [[unlikely]] {
         return make_unexpected(errc_type::operation_failure);
      }
      return {};
   }

 public:
   stream_writable_data_source() = delete;
   stream_writable_data_source(stream_writable_data_source const &) = delete;
   stream_writable_data_source(stream_writable_data_source &&rhs) = default;
   template <class... Args>
      requires(!std::same_as<std::remove_cvref_t<Args>,
                             stream_writable_data_source> ||
               ...)
   explicit stream_writable_data_source(Args &&...args)
       : stream_data_source<value_type>(std::forward<Args>(args)...) {}
   ~stream_writable_data_source() = default;

   template <class Self>
   [[nodiscard]] auto write(this Self &self, region reg,
                            std::span<std::byte const> data)
       -> result_type<void> {
      assert(reg.size <= std::numeric_limits<std::streamsize>::max());
      if (!self.data) [[unlikely]] {
         if (self.data.bad()) [[unlikely]] {
            return make_unexpected(errc_type::invalid_member_variable);
         }
         self.data.clear();
      }
      if (data.size() < reg.size) [[unlikely]] {
         return make_unexpected(errc_type::out_of_range);
      }
      if (auto result = self.seekp(reg.offset); !result) {
         return std::unexpected{std::move(result).error()};
      }
      self.data.write(reinterpret_cast<char const *>(data.data()), reg.size);
      if (self.data.bad()) [[unlikely]] {
         return make_unexpected(errc_type::invalid_member_variable);
      }
      if (self.data.fail()) [[unlikely]] {
         return make_unexpected(errc_type::operation_failure);
      }
      return {};
   }

   auto operator=(stream_writable_data_source const &rhs)
       -> stream_writable_data_source & = delete;
   auto operator=(stream_writable_data_source &&rhs)
       -> stream_writable_data_source & = default;
};

using sds = stream_data_source<std::istream>;
static_assert(data_source<sds>,
              "stream_data_source<std::istream> is data_source");
static_assert(!writable_data_source<sds>,
              "stream_data_source<std::istream> is not writable_data_source");

using swds = stream_writable_data_source<std::iostream>;
static_assert(data_source<swds>,
              "stream_writable_data_source<std::iostream> is data_source");
static_assert(
    writable_data_source<swds>,
    "stream_writable_data_source<std::iostream> is writable_data_source");

} // namespace maboroutu
#endif
