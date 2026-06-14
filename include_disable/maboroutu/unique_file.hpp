#pragma once

#include <cassert>
#include <corecrt.h>
#include <cstdarg>
#include <cstdio>
#include <memory>
namespace maboroutu {
// cstdioなのでSTLであるが、namespaceがstdではないため
// wrapのグローバルで記述します。
class unique_file {
public:
  struct deletor {
    void operator()(FILE *val) { ::fclose(val); }
  };
  using file_pointer = typename ::std::unique_ptr<::FILE, deletor>;

  enum class offset_flag : decltype(SEEK_SET) {
    seek_set = SEEK_SET,
    seek_cur = SEEK_CUR,
    seek_end = SEEK_END,

    begin = seek_set,
    current = seek_cur,
    end = seek_end,
  };

  /*
  // remove 	ファイルを削除する
  // rename 	ファイル名を変更、ファイルを移動する
  // tmpfile 	一時ファイルを生成する
  // tempname(tmpnam) 	一時ファイル名を生成する
  // fclose 	ファイルを閉じる
  // fopen 	ファイルを開く
  // fflush 	ファイルをフラッシュする
  // fprintf 	書式を指定してファイルに出力する
  // fscanf 	書式を指定してファイルから入力する
  // vfprintf 	可変引数リスト va_list
  を使用し、書式を指定してファイルに出力する
  // vfscanf 	可変引数リスト va_list
  を使用し、書式を指定してファイルから入力する	C++17
  // fgetc 	ファイルから1文字入力する
  // fgets 	ファイルからN文字入力する
  // fputc 	ファイルに1文字出力する
  // fputs 	ファイルにN文字出力する
  // fread 	ファイルからN文字読み込む
  // fwrite 	ファイルにN文字書き込む
  // fgetpos 	ファイルの現在位置を取得する
  // fseek 	ファイルの現在位置を移動する
  // fsetpos 	ファイルの現在位置を設定する
  // ftell 	ファイルの現在位置を取得する
  // rewind 	ファイルの現在位置を先頭に戻し、エラーや終端判定をクリアする
  // clearerr 	エラーをクリアする
  // feof 	ファイルが終端に到達したか判定する
  // ferror 	ファイルストリームがエラー状態かを判定する
  // perror 	システムエラーメッセージを出力する
  */
protected:
private:
private:
  file_pointer _file;

  // FILE_pointer fconv_unique(::FILE *File) { return FILE_pointer(File); }
protected:
public:
  unique_file() : _file(nullptr) {}
  unique_file(unique_file const &file) = delete;
  unique_file(unique_file &&file) noexcept : _file(::std::move(file._file)) {};
  ~unique_file() = default;

  auto operator=(unique_file const &) -> unique_file & = delete;
  auto operator=(unique_file &&value) noexcept -> unique_file & {
    _file = ::std::move(value._file);
    return *this;
  }

  constexpr auto release() noexcept -> file_pointer::pointer {
    return _file.release();
  }

  constexpr void swap(unique_file &value) noexcept { _file.swap(value._file); }

  [[nodiscard]] constexpr auto get() const noexcept -> file_pointer::pointer {
    return _file.get();
  }

  constexpr auto get_deleter() noexcept -> file_pointer::deleter_type & {
    return _file.get_deleter();
  }
  [[nodiscard]] constexpr auto get_deleter() const noexcept
      -> file_pointer::deleter_type const & {
    return _file.get_deleter();
  }

  auto size() noexcept -> size_t {
    long current = ftell();
    fseek(0L, offset_flag::end);
    size_t const ret_value = ftell();
    fseek(current, offset_flag::begin);
    return ret_value;
  }

  [[nodiscard]] static auto constexpr remove(const char *filename) noexcept
      -> int {
    return ::remove(filename);
  }
  [[nodiscard]] static auto constexpr rename(const char *old_filename,
                                             const char *new_filename) noexcept
      -> int {
    return ::rename(old_filename, new_filename);
  }
  [[nodiscard]] [[deprecated("Using tmpfile().")]] auto tmpfile() noexcept
      -> bool {
    _file.reset(::tmpfile());
    return _file.operator bool();
  }
  [[nodiscard]] auto tmpfile_s() noexcept -> errno_t {
    ::FILE *file = nullptr;
    errno_t ret_value = ::tmpfile_s(&file);
    if (ret_value != 0) {
      return ret_value;
    }
    _file.reset(file);
    return ret_value;
  }
  [[nodiscard]] [[deprecated("Using tmpnam().")]] static constexpr auto
  tmpnam(char *s) noexcept -> char * {
    return ::tmpnam(s);
  }
  [[nodiscard]] static auto constexpr tmpnam_s(char *s,
                                               rsize_t maxsize) noexcept
      -> errno_t {
    return ::tmpnam_s(s, maxsize);
  }
  [[nodiscard]] auto constexpr fclose() noexcept -> int {
    return ::fclose(_file.release());
  }
  [[nodiscard]] auto constexpr fflush() noexcept -> int {
    return ::fflush(_file.get());
  }
  [[nodiscard]] [[deprecated("Using fopen.")]] auto constexpr fopen(
      const char *filename, const char *mode) noexcept -> bool {
    _file.reset(::fopen(filename, mode));
    if (_file == nullptr) [[unlikely]] {
      return false;
    }
    return true;
  }
  [[nodiscard]] auto constexpr fopen_s(const char *filename,
                                       const char *mode) noexcept -> errno_t {
    ::FILE *file = nullptr;
    errno_t ret_value = ::fopen_s(&file, filename, mode);
    if (ret_value != 0) [[unlikely]] {
      return ret_value;
    }
    _file.reset(file);
    return ret_value;
  }
  auto fprintf(const char *format, ...) noexcept -> int {
    ::va_list va_list = nullptr;
    va_start(va_list, format);
    auto ret_value = ::vfprintf(_file.get(), format, va_list);
    va_end(va_list);
    return ret_value;
  }
  auto fscanf(const char *format, ...) noexcept -> int {
    ::va_list va_list = nullptr;
    va_start(va_list, format);
    auto ret_value = ::vfscanf(_file.get(), format, va_list);
    va_end(va_list);
    return ret_value;
  }
  auto constexpr vfprintf(const char *format, ::va_list arg) noexcept -> int {
    return ::vfprintf(_file.get(), format, arg);
  }
  auto constexpr vfscanf(const char *format, va_list arg) noexcept -> int {
    return ::vfscanf(_file.get(), format, arg);
  }
  auto constexpr fgetc() noexcept -> int { return ::fgetc(_file.get()); }
  auto constexpr fputc(int c) noexcept -> int {
    return ::fputc(c, _file.get());
  }
  auto constexpr fputs(const char *s) noexcept -> int {
    return ::fputs(s, _file.get());
  }
  /**
   * @brief [::std::fread wrap.]
   *
   * @param Ptr [(ptr)dst value]
   * @param TypeSize [(size)dst type size.(exp 'Ptr = &uint16_t' when TypeSize =
   * sizeof(uint16_t).)]
   * @param Size [(nmemb)Array size.]
   * @return [TODO:return]
   */
  auto constexpr fread(void *ptr, size_t type_size, size_t size) noexcept
      -> size_t {
    return ::fread(ptr, type_size, size, _file.get());
  }
  /**
   * @brief [::std::fwrite wrap.]
   *
   * @param Ptr [(ptr)src value]
   * @param TypeSize [(size)src type size.(exp 'Ptr = &uint16_t' when TypeSize =
   * sizeof(uint16_t).)]
   * @param Size [(nmemb)Array size.]
   * @return [TODO:return]
   */
  auto constexpr fwrite(const void *ptr, size_t type_size, size_t size) noexcept
      -> size_t {
    return ::fwrite(ptr, type_size, size, _file.get());
  }

  auto constexpr fgetpos() noexcept -> fpos_t {
    ::fpos_t ret_value = NULL;
    ::fgetpos(_file.get(), &ret_value);
    return ret_value;
  }

  auto constexpr fseek(long offset, offset_flag offset_flag) noexcept -> int {
    return ::fseek(_file.get(), offset, (int)offset_flag);
  }

  auto constexpr fsetpos(fpos_t pos) noexcept -> int {
    return ::fsetpos(_file.get(), &pos);
  }
  [[nodiscard]] long int constexpr ftell() noexcept {
    return ::ftell(_file.get());
  }
  void constexpr rewind() noexcept { ::rewind(_file.get()); }
  void constexpr clearerr() noexcept { ::clearerr(_file.get()); }
  [[nodiscard]] auto constexpr feof() noexcept -> int {
    return ::feof(_file.get());
  }
  [[nodiscard]] auto constexpr ferror() noexcept -> int {
    return ::ferror(_file.get());
  }
  static void constexpr perror(const char *s) noexcept { ::perror(s); }

  // operator
  explicit operator bool() const noexcept { return _file.get() != nullptr; }
};
} // namespace maboroutu
