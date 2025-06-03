#pragma once
#include <cstdarg>
#include <cstdio>

namespace maboroutu {

class UniqueFileSeekInterface {
public:
  enum class OffsetFlag : int {
    SeekSet = SEEK_SET,
    SeekCur = SEEK_CUR,
    SeekEnd = SEEK_END,

    Begin = SeekSet,
    Current = SeekCur,
    End = SeekEnd,
  };

private:
  virtual int i_fgetpos(fpos_t *Pos) = 0;
  virtual int i_fseek(long Offset, OffsetFlag OffsetFlag) = 0;

public:
  int constexpr fgetpos(fpos_t *Pos) noexcept { return i_fgetpos(Pos); };
  int constexpr fseek(long Offset, OffsetFlag OffsetFlag) noexcept {
    return i_fseek(Offset, OffsetFlag);
  };
};

class UniqueFileInputInterface {
private:
  virtual size_t i_fread(void *Ptr, size_t TypeSize, size_t Size) = 0;

public:
  size_t constexpr fread(void *Ptr, size_t TypeSize, size_t Size) noexcept {
    return i_fread(Ptr, TypeSize, Size);
  }
};

class UniqueFileOutputInterface {
private:
  virtual size_t i_fwrite(const void *Ptr, size_t TypeSize, size_t Size) = 0;

public:
  size_t constexpr fwrite(const void *Ptr, size_t TypeSize,
                          size_t Size) noexcept {
    return i_fwrite(Ptr, TypeSize, Size);
  }
};

class UniqueFileTextInputInterface {
private:
  virtual int i_vfscanf(const char *Format, va_list Arg) = 0;

public:
  int fscanf(const char *Format, ...) noexcept {
    ::va_list VaArg;
    va_start(VaArg, Format);
    auto RetValue = vfscanf(Format, VaArg);
    va_end(VaArg);
    return RetValue;
  }
  int vfscanf(const char *Format, va_list Arg) {
    return i_vfscanf(Format, Arg);
  }
};

class UniqueFileTextOutputInterface {
private:
  virtual int i_vfprintf(const char *Format, ::va_list Arg) = 0;

public:
  int vfprintf(const char *Format, ...) noexcept {
    ::va_list VaArg;
    va_start(VaArg, Format);
    auto RetValue = vfprintf(Format, VaArg);
    va_end(VaArg);
    return RetValue;
  }
  int constexpr vfprintf(const char *Format, va_list Arg) noexcept {
    return i_vfprintf(Format, Arg);
  }
};

} // namespace maboroutu
