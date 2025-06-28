#pragma once

#include <concepts>
#include <cstddef>
namespace maboroutu {

template <class T>
concept IsIndexedTreeNodeContainer = requires(T &V) {
  T::key_type;
  std::integral<typename T::mapped_type>;
  { V.at(T::key_type()) } -> std::same_as<typename T::mapped_type>;
  { V.operator[](T::key_type()) } -> std::same_as<typename T::mapped_type>;
  { V.insert(T::value_type()) };
};

template <class T, IsIndexedTreeNodeContainer ContainerT>
struct IndexedTreeNode {
public:
  using value_type = T;
  using container_type = ContainerT;

public:
  value_type Value;
  container_type Entry;
};

} // namespace maboroutu
