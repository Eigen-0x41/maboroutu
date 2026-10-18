// 長さ不明の本体を書いた後で、先頭の長さフィールドを確定値で書き戻す例。
// reserve_patch は、末尾にプレースホルダを確保するRAIIオブジェクトを返す。
// 破棄までに resolve()（値を確定）または abandon()（放棄）を呼ぶ必要がある。
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>

import maboroutu.binary_layout;
import maboroutu.memory_data_buffer;

int main() {
   maboroutu::memory_data_buffer buffer;

   // 3バイト（24bit、ビッグエンディアン）の長さフィールドを予約する。
   auto patch = maboroutu::reserve_patch<3, maboroutu::endian::big>(buffer);
   if (!patch) {
      return 1;
   }

   // 本体を書く。
   constexpr std::array<std::byte, 5> body{std::byte{'h'}, std::byte{'e'},
                                           std::byte{'l'}, std::byte{'l'},
                                           std::byte{'o'}};
   if (!buffer.append(std::span<std::byte const>(body))) {
      patch->abandon();
      return 1;
   }

   // 本体の長さを確定して、予約位置へ書き戻す。
   if (!patch->resolve(static_cast<std::uint32_t>(body.size()))) {
      return 1;
   }

   auto const length =
       maboroutu::read_uint<3, maboroutu::endian::big>(buffer, 0);
   if (!length) {
      return 1;
   }
   std::printf("length field = %llu\n",
               static_cast<unsigned long long>(*length));
   return 0;
}
