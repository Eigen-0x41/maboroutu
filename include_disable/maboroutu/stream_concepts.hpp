#pragma once

#include <concepts>
#include <cstdarg>
#include <cstdio>
namespace maboroutu {
template <class T>
concept stream_pos_concept = requires(T &value) {
  typename T::OffsetFlag;
  { value.fgetpos() } -> std::same_as<fpos_t>;
  { value.fsetpos((fpos_t)(0)) } -> std::same_as<int>;
};

template <class T>
concept stream_input_concept = requires(T &value) {
  {
    value.fread((void *)(nullptr), size_t(), size_t())
  } -> std::same_as<size_t>;
};
template <class T>
concept stream_output_concept = requires(T &value) {
  {
    value.fwrite((void *)(nullptr), size_t(), size_t())
  } -> std::same_as<size_t>;
};
template <class T>
concept stream_io_concept = stream_input_concept<T> &&
                            stream_output_concept<T> && stream_pos_concept<T>;

template <class T>
concept stream_text_input_concept = requires(T &value) {
  // TODO: 0~x個の引数となっている部分は可変長引数。
  // 綺麗に判別できる方法があるなら変えるべき。
  { value.fscanf("") } -> std::same_as<int>;
  { value.fscanf("", nullptr) } -> std::same_as<int>;
  { value.fscanf("", nullptr, nullptr) } -> std::same_as<int>;
  { value.fscanf("", nullptr, nullptr, nullptr) } -> std::same_as<int>;
  { value.vfscanf("", ::va_list()) } -> std::same_as<int>;
};
template <class T>
concept stream_text_output_concept = requires(T &value) {
  // TODO: 0~x個の引数となっている部分は可変長引数。
  // 綺麗に判別できる方法があるなら変えるべき。
  { value.fprintf("") } -> std::same_as<int>;
  { value.fprintf("", nullptr) } -> std::same_as<int>;
  { value.fprintf("", nullptr, nullptr) } -> std::same_as<int>;
  { value.fprintf("", nullptr, nullptr, nullptr) } -> std::same_as<int>;
  { value.vfprintf("", ::va_list()) } -> std::same_as<int>;
};

template <class T>
concept stream_text_io_concept =
    stream_text_input_concept<T> && stream_text_output_concept<T>;

} // namespace maboroutu
