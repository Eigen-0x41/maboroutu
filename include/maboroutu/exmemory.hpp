// #pragma once
//
// #include <concepts>
// #include <cstddef>
// #include <cstdio>
// #include <cstdlib>
// #include <memory>
// #include <new>
// #include <type_traits>
// namespace maboroutu {
// struct free_delete {
//   void operator()(void *Ptr) { free(Ptr); }
// };
// template <class T> struct operator_new_deletor {
//   void operator()(T *Instance) {
//     Instance->~T();
//     ::operator delete(reinterpret_cast<void *>(Instance));
//   }
// };
// struct fclose_delete {
//   void operator()(FILE *File) { ::fclose(File); }
// };
// } // namespace maboroutu
//
// namespace std {
// /// 初期化なし。
// // a_Size byte 分のメモリを確保。
// template <class T>
// unique_ptr<T, maboroutu::operator_new_deletor<T>>
// make_unique_operator_new(size_t Size) {
//   unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
//   RetVal.reset(reinterpret_cast<T *>(::operator new(Size)));
//   return RetVal;
// }
// // a_Size byte 分のメモリを a_Alignment の要件を満たして確保。
// template <class T>
// unique_ptr<T, maboroutu::operator_new_deletor<T>>
// make_unique_operator_new(size_t Size, std::align_val_t Alignment) {
//   unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
//   RetVal.reset(reinterpret_cast<T *>(::operator new(Size, Alignment)));
//   return RetVal;
// }
// // a_Size byte 分のメモリを確保。
// //  リソースの確保は保証されない。
// template <class T>
// unique_ptr<T, maboroutu::operator_new_deletor<T>>
// make_unique_operator_new(size_t Size, const std::nothrow_t &) {
//   unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
//   RetVal.reset(reinterpret_cast<T *>(::operator new(Size)));
//   return RetVal;
// }
// // a_Size byte 分のメモリを a_Alignment の要件を満たして確保。
// // リソースの確保は保証されない。
// template <class T>
// unique_ptr<T, maboroutu::operator_new_deletor<T>>
// make_unique_operator_new(size_t Size, std::align_val_t Alignment,
//                          const std::nothrow_t &) {
//   unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
//   RetVal.reset(reinterpret_cast<T *>(::operator new(Size, Alignment)));
//   return RetVal;
// }
//
// /// 初期化あり。
// // a_Size byte 分のメモリを確保。
// template <class T, class... ArgsT>
// unique_ptr<T, maboroutu::operator_new_deletor<T>>
// make_unique_new(size_t Size, ArgsT &&...Args) {
//   unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
//   RetVal.reset(reinterpret_cast<T *>(::operator new(Size)));
//   new (RetVal.get()) T(std::forward(Args)...);
//   return RetVal;
// }
// // a_Size byte 分のメモリを a_Alignment の要件を満たして確保。
// template <class T, class... ArgsT>
// unique_ptr<T, maboroutu::operator_new_deletor<T>>
// make_unique_new(size_t Size, std::align_val_t Alignment, ArgsT &&...Args) {
//   unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
//   RetVal.reset(reinterpret_cast<T *>(::operator new(Size, Alignment)));
//   new (RetVal.get()) T(std::forward(Args)...);
//   return RetVal;
// }
// // a_Size byte 分のメモリを確保。
// // リソースの確保は保証されない。
// template <class T, class... ArgsT>
// unique_ptr<T, maboroutu::operator_new_deletor<T>>
// make_unique_new(size_t Size, const std::nothrow_t &, ArgsT &&...Args) {
//   unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
//   RetVal.reset(reinterpret_cast<T *>(::operator new(Size)));
//   new (RetVal.get()) T(std::forward(Args)...);
//   return RetVal;
// }
// // a_Size byte 分のメモリを a_Alignment の要件を満たして確保。
// // リソースの確保は保証されない。
// template <class T, class... ArgsT>
// unique_ptr<T, maboroutu::operator_new_deletor<T>>
// make_unique_new(size_t Size, std::align_val_t Alignment, const std::nothrow_t &,
//                 ArgsT &&...Args) {
//   unique_ptr<T, maboroutu::operator_new_deletor<T>> RetVal;
//   RetVal.reset(reinterpret_cast<T *>(::operator new(Size, Alignment)));
//   new (RetVal.get()) T(std::forward(Args)...);
//   return RetVal;
// }
// } // namespace std
//
// namespace maboroutu {
// template <class T, class Deleter, class... ArgsT>
// std::unique_ptr<T, Deleter>
// restruct_unique(std::unique_ptr<T, Deleter> &&UniquePtr, ArgsT &&...Args) {
//   UniquePtr.get()->~T();
//   return new (UniquePtr.get()->~T()) T(Args...);
// }
// } // namespace maboroutu
//
// ///  コンストラクタを呼ぶと値の初期化が入るため実装しません。
// // template <class T, class... Args>
// // unique_ptr<T, maboroutu::operator_new_deletor>
// // make_unique_for_overwrite(size_t a_Size) {
// //   unique_ptr<T, maboroutu::operator_new_deletor> r_Val =
// //       reinterpret_cast<T *>(::operator new(a_Size));
// //   return r_Val;
// // }
