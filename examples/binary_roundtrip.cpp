// memory_data_buffer へ数値を書き込み、読み戻す最小の例。
#include <cstdint>
#include <cstdio>
#include <vector>

import maboroutu.binary_convert;
import maboroutu.memory_data_buffer;

int main() {
   maboroutu::memory_data_buffer buffer;

   // 末尾に 4 + 8 + 12 バイトの領域を確保する。
   auto const area = buffer.grow(24);
   if (!area) {
      return 1;
   }

   // 32bit整数（ビッグエンディアン）と、64bit浮動小数点（リトルエンディアン）。
   auto const w1 =
       maboroutu::write_value<maboroutu::endian::big, std::uint32_t>(
           buffer, area->offset, 0x01020304u);
   auto const w2 = maboroutu::write_value<maboroutu::endian::little, double>(
       buffer, area->offset + 4, 3.5);
   // int の配列（リトルエンディアン）。
   auto const w3 = maboroutu::write_vector<maboroutu::endian::little, int>(
       buffer, area->offset + 12, std::vector<int>{10, 20, 30});
   if (!w1 || !w2 || !w3) {
      return 1;
   }

   auto const r1 = maboroutu::read_value<maboroutu::endian::big, std::uint32_t>(
       buffer, area->offset);
   auto const r2 = maboroutu::read_value<maboroutu::endian::little, double>(
       buffer, area->offset + 4);
   auto const r3 = maboroutu::read_vector<maboroutu::endian::little, int>(
       buffer, area->offset + 12, 3);
   if (!r1 || !r2 || !r3) {
      return 1;
   }

   std::printf("u32 = 0x%08x, double = %g, ints = %d %d %d\n", *r1, *r2,
               (*r3)[0], (*r3)[1], (*r3)[2]);
   return 0;
}
