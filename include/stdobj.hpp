#pragma once

#include "uniqueFileInterface.hpp"
#include <corecrt.h>
#include <cstdarg>
#include <cstdio>
#include <memory>

namespace maboroutu {
namespace wrap {
// cstdioなのでSTLであるが、namespaceがstdではないため
// wrapのグローバルで記述します。
class UniqueFile : private UniqueFileSeekInterface,
                   private UniqueFileInputInterface,
                   private UniqueFileOutputInterface,
                   private UniqueFileTextInputInterface,
                   private UniqueFileTextOutputInterface {
private:
  struct Deletor {
    void operator()(FILE *Val) { ::fclose(Val); }
  };

public:
  using FILE_pointer = typename ::std::unique_ptr<::FILE, Deletor>;

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
private:
  int i_fgetpos(fpos_t *Pos) override { return fgetpos(Pos); }
  int i_fseek(long Offset, OffsetFlag OffsetFlag) override {
    return fseek(Offset, OffsetFlag);
  }
  size_t i_fread(void *Ptr, size_t TypeSize, size_t Size) override {
    return fread(Ptr, TypeSize, Size);
  }
  size_t i_fwrite(const void *Ptr, size_t TypeSize, size_t Size) override {
    return fwrite(Ptr, TypeSize, Size);
  }
  int i_vfscanf(const char *Format, va_list Arg) override {
    return vfscanf(Format, Arg);
  }
  int i_vfprintf(const char *Format, ::va_list Arg) override {
    return vfprintf(Format, Arg);
  }

protected:
  FILE_pointer Continer;

  // FILE_pointer fconv_unique(::FILE *File) { return FILE_pointer(File); }

public:
  UniqueFile() : Continer(nullptr) {}
  UniqueFile(UniqueFile const &File) = delete;
  UniqueFile(UniqueFile &&File) : Continer(::std::move(File.Continer)) {};
  ~UniqueFile() = default;

  UniqueFile &operator=(UniqueFile const &) = delete;
  UniqueFile &operator=(UniqueFile &&Value) {
    Continer = ::std::move(Value.Continer);
    return *this;
  }

  [[nodiscard]] static int constexpr remove(const char *Filename) noexcept {
    return ::remove(Filename);
  }
  [[nodiscard]] static int constexpr rename(const char *OldFilename,
                                            const char *NewFilename) noexcept {
    return ::rename(OldFilename, NewFilename);
  }
  [[nodiscard]] [[deprecated("Using tmpfile().")]] bool tmpfile(void) noexcept {
    Continer.reset(::tmpfile());
    return Continer.operator bool();
  }
  [[nodiscard]] errno_t tmpfile_s(void) noexcept {
    ::FILE *File;
    errno_t RetValue = ::tmpfile_s(&File);
    if (RetValue) {
      return RetValue;
    }
    Continer.reset(File);
    return RetValue;
  }
  [[nodiscard]] [[deprecated("Using tmpnam().")]] static char constexpr *
  tmpnam(char *S) noexcept {
    return ::tmpnam(S);
  }
  [[nodiscard]] static errno_t constexpr tmpnam_s(char *S,
                                                  rsize_t Maxsize) noexcept {
    return ::tmpnam_s(S, Maxsize);
  }
  [[nodiscard]] int constexpr fclose() noexcept {
    return ::fclose(Continer.release());
  }
  [[nodiscard]] int constexpr fflush() noexcept {
    return ::fflush(Continer.get());
  }
  [[nodiscard]] [[deprecated("Using fopen.")]] bool constexpr fopen(
      const char *Filename, const char *Mode) noexcept {
    Continer.reset(::fopen(Filename, Mode));
    return Continer != nullptr;
  }
  [[nodiscard]] errno_t constexpr fopen_s(const char *Filename,
                                          const char *Mode) noexcept {
    ::FILE *File;
    errno_t RetValue = ::fopen_s(&File, Filename, Mode);
    if (RetValue) {
      return RetValue;
    }
    Continer.reset(File);
    return RetValue;
  }
  int fprintf(const char *Format, ...) noexcept {
    ::va_list VaList;
    va_start(VaList, Format);
    auto RetValue = ::vfprintf(Continer.get(), Format, VaList);
    va_end(VaList);
    return RetValue;
  }
  int fscanf(const char *Format, ...) noexcept {
    ::va_list VaList;
    va_start(VaList, Format);
    auto RetValue = ::vfscanf(Continer.get(), Format, VaList);
    va_end(VaList);
    return RetValue;
  }
  int constexpr vfprintf(const char *Format, ::va_list Arg) noexcept {
    return ::vfprintf(Continer.get(), Format, Arg);
  }
  int constexpr vfscanf(const char *Format, va_list Arg) noexcept {
    return ::vfscanf(Continer.get(), Format, Arg);
  }
  int constexpr fgetc() noexcept { return ::fgetc(Continer.get()); }
  int constexpr fputc(int C) noexcept { return ::fputc(C, Continer.get()); }
  int constexpr fputs(const char *S) noexcept {
    return ::fputs(S, Continer.get());
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
  size_t constexpr fread(void *Ptr, size_t TypeSize, size_t Size) noexcept {
    return ::fread(Ptr, TypeSize, Size, Continer.get());
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
  size_t constexpr fwrite(const void *Ptr, size_t TypeSize,
                          size_t Size) noexcept {
    return ::fwrite(Ptr, TypeSize, Size, Continer.get());
  }
  int constexpr fgetpos(fpos_t *Pos) noexcept {
    return ::fgetpos(Continer.get(), Pos);
  }

  using OffsetFlag = UniqueFileSeekInterface::OffsetFlag;
  int constexpr fseek(long Offset, OffsetFlag OffsetFlag) noexcept {
    return ::fseek(Continer.get(), Offset, (int)OffsetFlag);
  }

  int constexpr fsetpos(const fpos_t *Pos) noexcept {
    return ::fsetpos(Continer.get(), Pos);
  }
  [[nodiscard]] long int constexpr ftell() noexcept {
    return ::ftell(Continer.get());
  }
  void constexpr rewind() noexcept { return ::rewind(Continer.get()); }
  void constexpr clearerr() noexcept { return ::clearerr(Continer.get()); }
  [[nodiscard]] int constexpr feof() noexcept { return ::feof(Continer.get()); }
  [[nodiscard]] int constexpr ferror() noexcept {
    return ::ferror(Continer.get());
  }
  static void constexpr perror(const char *S) noexcept { return ::perror(S); }

  // operator
  explicit operator bool() const noexcept { return Continer.get() != nullptr; }
};
} // namespace wrap
} // namespace maboroutu
