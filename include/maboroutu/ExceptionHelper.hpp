#pragma once

#include <string>
#include <string_view>
#include <vcruntime_typeinfo.h>
namespace maboroutu {
template <class T>
static inline std::string
exceptionMessageCreater(T *RunClassPtr, std::string_view const FuncName,
                        std::string_view const Msg) {
  std::string ErrorMessage = "ERROR= ";
  ErrorMessage += typeid(*RunClassPtr).name();
  ErrorMessage += ".";
  ErrorMessage += FuncName;
  ErrorMessage += "(): ";
  ErrorMessage += Msg;
  return ErrorMessage;
}

static inline std::string
exceptionMessageCreater(std::string_view const FuncName,
                        std::string_view const Msg) {
  std::string ErrorMessage = "ERROR= ";
  ErrorMessage += FuncName;
  ErrorMessage += "(): ";
  ErrorMessage += Msg;
  return ErrorMessage;
}
} // namespace maboroutu
