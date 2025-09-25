#pragma once

#include "maboroutu/error.hpp"
#include "maboroutuDef.hpp"

using byte_t = typename maboroutu::byte_t;
using binary_t = typename maboroutu::binary_t;

using exception = typename maboroutu::exception;
template <class Ty = void, maboroutu::ExceptionConcepts Err = exception>
using ret = typename maboroutu::ret<Ty, Err>;
using retErr = typename std::unexpected<exception>;

template <maboroutu::ExceptionConcepts ErrT, class... ArgsT>
retErr makeRetErr(ArgsT &&...Args) {
  return maboroutu::makeRetErr<ErrT, ArgsT...>(std::forward<ArgsT>(Args)...);
}

template <class T> retErr convRetErr(ret<T> const &Ret) {
  return maboroutu::convRetErr<T>(Ret);
}

template <class T> using mut_ref_t = maboroutu::mut_ref_t<T>;
