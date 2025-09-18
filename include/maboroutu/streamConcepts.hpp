#pragma once

#include <concepts>
#include <cstdarg>
#include <cstdio>
namespace maboroutu {
template <class T>
concept StreamPosConcept = requires(T &V) {
  typename T::OffsetFlag;
  { V.fgetpos() } -> std::same_as<fpos_t>;
  { V.fsetpos((fpos_t)(0)) } -> std::same_as<int>;
};

template <class T>
concept StreamInputConcept = requires(T &V) {
  { V.fread((void *)(nullptr), size_t(), size_t()) } -> std::same_as<size_t>;
};
template <class T>
concept StreamOutputConcept = requires(T &V) {
  { V.fwrite((void *)(nullptr), size_t(), size_t()) } -> std::same_as<size_t>;
};
template <class T>
concept StreamIOConcept =
    StreamInputConcept<T> && StreamOutputConcept<T> && StreamPosConcept<T>;

template <class T>
concept StreamTextInputConcept = requires(T &V) {
  // TODO: 0~x個の引数となっている部分は可変長引数。
  // 綺麗に判別できる方法があるなら変えるべき。
  { V.fscanf("") } -> std::same_as<int>;
  { V.fscanf("", nullptr) } -> std::same_as<int>;
  { V.fscanf("", nullptr, nullptr) } -> std::same_as<int>;
  { V.fscanf("", nullptr, nullptr, nullptr) } -> std::same_as<int>;
  { V.vfscanf("", ::va_list()) } -> std::same_as<int>;
};
template <class T>
concept StreamTextOutputConcept = requires(T &V) {
  // TODO: 0~x個の引数となっている部分は可変長引数。
  // 綺麗に判別できる方法があるなら変えるべき。
  { V.fprintf("") } -> std::same_as<int>;
  { V.fprintf("", nullptr) } -> std::same_as<int>;
  { V.fprintf("", nullptr, nullptr) } -> std::same_as<int>;
  { V.fprintf("", nullptr, nullptr, nullptr) } -> std::same_as<int>;
  { V.vfprintf("", ::va_list()) } -> std::same_as<int>;
};

template <class T>
concept StreamTextIOConcept =
    StreamTextInputConcept<T> && StreamTextOutputConcept<T>;

} // namespace maboroutu
