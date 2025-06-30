#pragma once

namespace maboroutu {
template <class T>
concept StencilConcepts = requires(T &V) {
  { V(binary(), size_t()) };
};
} // namespace maboroutu
