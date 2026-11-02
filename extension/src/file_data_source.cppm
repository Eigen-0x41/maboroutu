module;

#if (__STDC_HOSTED__ != 0) ||                                                  \
    defined(MABOROUTU_FILE_DATA_SOURCE_ALLOW_FREESTANDING)
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <filesystem>
#include <limits>
#include <memory>
#include <span>
#include <utility>
#if defined(_WIN32)
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#elif __has_include(<unistd.h>)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#endif

export module maboroutu.file_data_source;

#if (__STDC_HOSTED__ == 0) &&                                                  \
    !defined(MABOROUTU_FILE_DATA_SOURCE_ALLOW_FREESTANDING)
static_assert(
    false,
    "maboroutu.file_data_source depends on <cstdio> and <filesystem>, which "
    "are NOT part of any standardized freestanding library subset. This "
    "module is therefore disabled by default under freestanding "
    "implementations. "
    "If your freestanding implementation happens to provide conforming "
    "<cstdio> and <filesystem> (a non-standard extension), define "
    "MABOROUTU_FILE_DATA_SOURCE_ALLOW_FREESTANDING before importing this "
    "module to explicitly acknowledge that this is unverified by the "
    "standard, and proceed at your own risk. Actual runtime conformance "
    "is the responsibility of the library user.");
#else

export import maboroutu.data_source;

namespace maboroutu::file_detail {

// 64ビットのオフセットを扱えるseek/tell。標準のfseek/ftellはlongを取るため、
// longが32ビットの処理系（Windows等）では2GiB以上のファイルを扱えない。
#if defined(_WIN32)
using offset_type = long long;
inline auto seek(std::FILE *file, offset_type offset, int whence) noexcept
    -> int {
   return ::_fseeki64(file, offset, whence);
}
inline auto tell(std::FILE *file) noexcept -> offset_type {
   return ::_ftelli64(file);
}
#elif defined(_POSIX_VERSION)
using offset_type = off_t;
inline auto seek(std::FILE *file, offset_type offset, int whence) noexcept
    -> int {
   return ::fseeko(file, offset, whence);
}
inline auto tell(std::FILE *file) noexcept -> offset_type {
   return ::ftello(file);
}
#else
using offset_type = long;
inline auto seek(std::FILE *file, offset_type offset, int whence) noexcept
    -> int {
   return std::fseek(file, offset, whence);
}
inline auto tell(std::FILE *file) noexcept -> offset_type {
   return std::ftell(file);
}
#endif

/**
 * @brief offset が offset_type で表せるかを返す。
 */
[[nodiscard]] inline auto offset_fits(std::size_t offset) noexcept -> bool {
   return static_cast<std::uintmax_t>(offset) <=
          static_cast<std::uintmax_t>(std::numeric_limits<offset_type>::max());
}

/**
 * @brief 先頭から offset バイトの位置へ移動する。
 * @pre offset_fits(offset)
 * @return 移動できた場合は true。
 */
[[nodiscard]] inline auto seek_set(std::FILE *file,
                                   std::size_t offset) noexcept -> bool {
   return seek(file, static_cast<offset_type>(offset), SEEK_SET) == 0;
}

/**
 * @brief 開いているファイルの種類とサイズを、ハンドルから取得する。
 *
 * POSIX は fstat、Windows は _fstat64 を用いる（open() での種類の検査用）。パスは参照しない
 * （パスは、開いたファイルとは別のファイルを指すことがあるため）。
 * いずれでもない処理系では、標準の範囲で、末尾へのシークにより取得する
 * （この場合、ファイル位置が変わる。バイナリストリームが SEEK_END を
 * 意味のある形でサポートするとは、C標準は要求しない）。
 *
 * @return 取得できた場合は true。
 */
[[nodiscard]] inline auto stat_handle(std::FILE *file, std::uintmax_t &size,
                                      bool &regular) noexcept -> bool {
#if defined(_WIN32)
   struct __stat64 st;
   if (::_fstat64(::_fileno(file), &st) != 0 || st.st_size < 0) {
      return false;
   }
   regular = (st.st_mode & _S_IFMT) == _S_IFREG;
   size = static_cast<std::uintmax_t>(st.st_size);
   return true;
#elif defined(_POSIX_VERSION)
   struct stat st;
   if (::fstat(::fileno(file), &st) != 0 || st.st_size < 0) {
      return false;
   }
   regular = S_ISREG(st.st_mode);
   size = static_cast<std::uintmax_t>(st.st_size);
   return true;
#else
   if (seek(file, 0, SEEK_END) != 0) {
      return false;
   }
   auto const end = tell(file);
   if (end < 0) {
      return false;
   }
   regular = true;
   size = static_cast<std::uintmax_t>(end);
   return true;
#endif
}

/**
 * @brief 開いているファイルのサイズを、ハンドルから取得する（size() 用）。
 *
 * Windows では、_fstat64 が _filelengthi64 より大幅に遅いことをMSVCの実測で
 * 確認したため（仕様書 4.20 節）、_filelengthi64 を用いる。通常のファイルかどうかの
 * 判定は open() で一度だけ行い、ここでは行わない。他の環境では stat_handle と同じ。
 *
 * @return 取得できた場合は true。
 */
[[nodiscard]] inline auto size_handle(std::FILE *file,
                                      std::uintmax_t &size) noexcept -> bool {
#if defined(_WIN32)
   auto const length = ::_filelengthi64(::_fileno(file));
   if (length < 0) {
      return false;
   }
   size = static_cast<std::uintmax_t>(length);
   return true;
#else
   bool regular = false;
   return stat_handle(file, size, regular);
#endif
}

enum class mode_kind { read, update, truncate };

/**
 * @brief バイナリモードでファイルを開く。開けない場合は nullptr。
 *
 * Windows では、パスのワイド文字列で開く（_wfopen_s）。
 */
[[nodiscard]] inline auto open_file(std::filesystem::path const &path,
                                    mode_kind kind) noexcept -> std::FILE * {
#if defined(_WIN32)
   wchar_t const *mode = (kind == mode_kind::read)     ? L"rb"
                         : (kind == mode_kind::update) ? L"r+b"
                                                       : L"w+b";
   std::FILE *file = nullptr;
   if (::_wfopen_s(&file, path.c_str(), mode) != 0) {
      return nullptr;
   }
   return file;
#else
   char const *mode = (kind == mode_kind::read)     ? "rb"
                      : (kind == mode_kind::update) ? "r+b"
                                                    : "w+b";
   return std::fopen(path.c_str(), mode);
#endif
}

} // namespace maboroutu::file_detail

namespace maboroutu {

// [[file_open_mode]]
/**
 * @brief file_writable_data_source を開く方法。
 */
export enum class file_open_mode {
   /// 既存のファイルを、内容を保ったまま読み書き用に開く（存在しなければ失敗）。
   existing,
   /// ファイルを作成する。既存の場合は、内容を空にする。
   truncate,
};

// [[file_data_source]]
/**
 * @brief ファイルを data_source として扱う。
 *
 * C標準の std::FILE を、バイナリモードで開いて所有する（move-only）。
 * 破棄時に fclose する。通常のファイルのみを対象とする。
 *
 * 各操作は、先頭から指定位置へのシークで始まる。
 */
export class file_data_source {
 public: /*STRUCT_FIELD*/
   template <class T> using result_type = data_source_result<T>;

 private:
   using errc_type = result_type<void>::error_type::code_type;

   struct closer {
      void operator()(std::FILE *file) const noexcept { std::fclose(file); }
   };

 protected:
   /**
    * @brief file の所有権を受け取る。
    * @pre file != nullptr（違反は assert で検出する）
    */
   explicit file_data_source(std::FILE *file) noexcept : _file(file) {
      assert(file != nullptr);
   }

   /**
    * @brief 開いたファイルが通常のファイルであることを検査する。
    *
    * 通常のファイル以外（パイプ、デバイス等）は、ランダムアクセスできず、
    * サイズも意味を持たないため、invalid_operation とする。
    */
   [[nodiscard]] auto _check_regular() const noexcept -> result_type<void> {
      std::uintmax_t size = 0;
      bool regular = false;
      if (!file_detail::stat_handle(_file.get(), size, regular)) [[unlikely]] {
         return make_unexpected(errc_type::operation_failure);
      }
      if (!regular) [[unlikely]] {
         return make_unexpected(errc_type::invalid_operation);
      }
      return {};
   }
   /**
    * @brief FILE がエラー状態の場合は invalid_member_variable を返す。
    */
   [[nodiscard]] auto _check_state() const noexcept -> result_type<void> {
      if (std::ferror(_file.get()) != 0) [[unlikely]] {
         return make_unexpected(errc_type::invalid_member_variable);
      }
      return {};
   }
   /**
    * @brief region が総サイズに収まることを検証する（size() を呼ぶ）。
    */
   [[nodiscard]] auto _check_region(region reg) const -> result_type<void> {
      auto total = size();
      if (!total) [[unlikely]] {
         return std::unexpected{std::move(total).error()};
      }
      if (reg.offset > *total || reg.size > *total - reg.offset) [[unlikely]] {
         return make_unexpected(errc_type::out_of_range);
      }
      return {};
   }
   /**
    * @brief region を out の先頭へ読み取る。範囲の検証は、読み込みの結果で行う。
    *
    * 範囲外（ストリームの終端を超える）は、fread の不足として検出する。
    * バイナリストリームにヌル文字を付加する処理系は想定しない（仕様 4.20 節）。
    * ファイルのサイズは参照しない。
    */
   [[nodiscard]] auto _read_checked(region reg, std::byte *out)
       -> result_type<void> {
      if (reg.size == 0) {
         return {};
      }
      // NOTE: offset_type で表せない位置は、どのファイルにも存在しない。
      if (!file_detail::offset_fits(reg.offset)) [[unlikely]] {
         return make_unexpected(errc_type::out_of_range);
      }
      if (!file_detail::seek_set(_file.get(), reg.offset)) [[unlikely]] {
         return make_unexpected(errc_type::operation_failure);
      }
      _pending_write = false; // シークは、未書き出しのデータを書き出す。
      auto const count = std::fread(out, 1, reg.size, _file.get());
      if (count == reg.size) [[likely]] {
         return {};
      }
      // NOTE: 指定サイズを読み込む前にEOFに達した場合は、out_of_rangeとする。
      return make_unexpected(std::ferror(_file.get()) != 0
                                 ? errc_type::invalid_member_variable
                                 : errc_type::out_of_range);
   }

   std::unique_ptr<std::FILE, closer> _file;
   /// 書き出していない書き込みがあるか（size() の前に fflush が必要）。
   mutable bool _pending_write = false;

 public:
   file_data_source() = delete;
   file_data_source(file_data_source const &) = delete;
   file_data_source(file_data_source &&rhs) = default;
   ~file_data_source() = default;

   auto operator=(file_data_source const &rhs) -> file_data_source & = delete;
   auto operator=(file_data_source &&rhs) -> file_data_source & = default;

   /**
    * @brief path のファイルを、読み取り用に開く。
    *
    * @return 開けない場合は operation_failure、通常のファイルでない場合は
    *         invalid_operation。
    */
   [[nodiscard]] static auto open(std::filesystem::path const &path)
       -> result_type<file_data_source> {
      auto *file = file_detail::open_file(path, file_detail::mode_kind::read);
      if (file == nullptr) [[unlikely]] {
         return make_unexpected(errc_type::operation_failure);
      }
      file_data_source source(file);
      if (auto checked = source._check_regular(); !checked) [[unlikely]] {
         return std::unexpected{std::move(checked).error()};
      }
      return source;
   }

   /**
    * @brief 保持している FILE を返す（所有権は移らない）。
    */
   [[nodiscard]] auto get() const noexcept -> std::FILE * {
      return _file.get();
   }
   /**
    * @brief FILE の所有権を手放す。以降の fclose は呼び出し側の責務となる。
    *
    * 書き込みバッファの書き出しの失敗は、デストラクタの fclose では
    * 検出できない。検出が必要な場合は、release() して fclose の結果を確認する。
    */
   [[nodiscard]] auto release() noexcept -> std::FILE * {
      return _file.release();
   }

   [[nodiscard]] auto read(region reg) -> result_type<byte_array> {
      // NOTE: 確保の前に範囲を検証する。regionは外部入力に由来し得るため、
      //       未検証のsizeで確保すると巨大な確保が先に発生する。
      if (auto checked = _check_region(reg); !checked) [[unlikely]] {
         return std::unexpected{std::move(checked).error()};
      }
      byte_array ret_value{
          .value = std::make_unique<byte_array::value_type>(reg.size),
          .size = reg.size,
      };
      if (auto result = _read_checked(reg, ret_value.value.get());
          !result) [[unlikely]] {
         return std::unexpected{std::move(result).error()};
      }
      return ret_value;
   }

   /**
    * @brief region で指定した範囲を dst の先頭へ読み取る。
    *
    * 確保を伴わないため、事前にサイズを問い合わせない（read() は確保の前に
    * 検証するため、問い合わせる）。範囲外は、fread の戻り値の不足で検出する。
    * 失敗時の dst の内容は不定である（4.3節）。
    *
    * @pre dst.size() >= reg.size（呼び出し側の責務。違反は assert で検出）
    */
   [[nodiscard]] auto read_into(region reg, std::span<std::byte> dst)
       -> result_type<void> {
      assert(dst.size() >= reg.size);
      if (auto state = _check_state(); !state) [[unlikely]] {
         return std::unexpected{std::move(state).error()};
      }
      return _read_checked(reg, dst.data());
   }

   /**
    * @brief ファイルのバイト数を取得する。
    * 格納されているバイナリ値を変更しないことのみ保証する。
    *
    * 開いているファイルのハンドルから取得する（file_detail::stat_handle）。
    * 書き出していない書き込みがある場合は、先に fflush する。
    *
    * @note data_source_handle の size() は const である。ファイルの位置や
    *       バッファは論理的な状態に含まれない（各操作が先頭からのシークで
    *       始まる）ため、const とする。複数スレッドからの同時呼び出しは
    *       保証しない（1.8節）。
    */
   [[nodiscard]] auto size() const -> result_type<std::size_t> {
      if (auto state = _check_state(); !state) [[unlikely]] {
         return std::unexpected{std::move(state).error()};
      }
      if (_pending_write) {
         if (std::fflush(_file.get()) != 0) [[unlikely]] {
            return make_unexpected(errc_type::operation_failure);
         }
         _pending_write = false;
      }
      std::uintmax_t total = 0;
      if (!file_detail::size_handle(_file.get(), total)) [[unlikely]] {
         return make_unexpected(errc_type::operation_failure);
      }
      if (total > std::numeric_limits<std::size_t>::max()) [[unlikely]] {
         return make_unexpected(errc_type::operation_failure);
      }
      return static_cast<std::size_t>(total);
   }
};

// [[file_writable_data_source]]
/**
 * @brief ファイルを writable_data_source として扱う（読み書き両用）。
 */
export class file_writable_data_source : public file_data_source {
 private:
   using errc_type = result_type<void>::error_type::code_type;

   explicit file_writable_data_source(std::FILE *file) noexcept
       : file_data_source(file) {}

 public:
   file_writable_data_source() = delete;
   file_writable_data_source(file_writable_data_source const &) = delete;
   file_writable_data_source(file_writable_data_source &&rhs) = default;
   ~file_writable_data_source() = default;

   auto operator=(file_writable_data_source const &rhs)
       -> file_writable_data_source & = delete;
   auto operator=(file_writable_data_source &&rhs)
       -> file_writable_data_source & = default;

   /**
    * @brief path のファイルを、読み書き用に開く。
    *
    * file_data_source::open(path)（読み取り専用）を隠す。
    *
    * @return 開けない場合は operation_failure、通常のファイルでない場合は
    *         invalid_operation。
    */
   [[nodiscard]] static auto open(std::filesystem::path const &path,
                                  file_open_mode mode)
       -> result_type<file_writable_data_source> {
      auto *file = file_detail::open_file(
          path, mode == file_open_mode::existing
                    ? file_detail::mode_kind::update
                    : file_detail::mode_kind::truncate);
      if (file == nullptr) [[unlikely]] {
         return make_unexpected(errc_type::operation_failure);
      }
      file_writable_data_source source(file);
      if (auto checked = source._check_regular(); !checked) [[unlikely]] {
         return std::unexpected{std::move(checked).error()};
      }
      return source;
   }

   /**
    * @brief region の位置へ data を書き込む。
    *
    * 現在のファイルサイズを超える位置への書き込みの結果（間の内容）は、
    * C標準では規定されない（POSIXとWindowsでは0で埋められる）。
    * 書き込みは、バッファに残ることがある。size()・read()・read_into() は、
    * 書き込みを反映した結果を返す。
    *
    * @pre data.size() == reg.size（呼び出し側の責務。違反は assert で検出）
    */
   [[nodiscard]] auto write(region reg, std::span<std::byte const> data)
       -> result_type<void> {
      assert(data.size() == reg.size);
      if (auto state = _check_state(); !state) [[unlikely]] {
         return std::unexpected{std::move(state).error()};
      }
      if (reg.size == 0) {
         return {};
      }
      if (!file_detail::offset_fits(reg.offset) ||
          !file_detail::seek_set(_file.get(), reg.offset)) [[unlikely]] {
         return make_unexpected(errc_type::operation_failure);
      }
      _pending_write = false;
      if (std::fwrite(data.data(), 1, reg.size, _file.get()) != reg.size)
          [[unlikely]] {
         return make_unexpected(std::ferror(_file.get()) != 0
                                    ? errc_type::invalid_member_variable
                                    : errc_type::operation_failure);
      }
      _pending_write = true;
      return {};
   }
};

static_assert(data_source<file_data_source>,
              "file_data_source is data_source");
static_assert(!writable_data_source<file_data_source>,
              "file_data_source is not writable_data_source");
static_assert(data_source<file_writable_data_source>,
              "file_writable_data_source is data_source");
static_assert(writable_data_source<file_writable_data_source>,
              "file_writable_data_source is writable_data_source");

} // namespace maboroutu
#endif
