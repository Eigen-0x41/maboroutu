// 結果型（std::expected ベース）とエラーメッセージの例。
// 範囲外の読み取りは、例外ではなく結果型のエラーとして返る。
#include <cstdint>
#include <cstdio>

import maboroutu.binary_convert;
import maboroutu.memory_data_buffer;

int main() {
   maboroutu::memory_data_buffer buffer; // 空のバッファ

   auto const value =
       maboroutu::read_value<maboroutu::endian::big, std::uint32_t>(buffer, 0);
   if (value) {
      return 1;
   }

   auto const &error = value.error();
   // コードの比較。
   bool const out_of_range =
       error.code() == maboroutu::errc::data_source::out_of_range;
   // コードに対応する固定文字列（std::string_view）。
   std::printf("out_of_range = %s, message = \"%.*s\"\n",
               out_of_range ? "true" : "false",
               static_cast<int>(error.message().size()),
               error.message().data());
   return 0;
}
