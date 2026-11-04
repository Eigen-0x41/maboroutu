// ファイルへ、2種類のバイナリ構造を書き込む例（読み込みは file_read.cpp）。
//
//   write_tree  ... 木構造（入れ子のノード）を、順番に書く。
//   write_table ... 文字列と、それを指すオフセットの表を書く。
//                   先頭の「表の位置」は、表を書くまで確定しないため、
//                   binary_layout_handle の deferred_resolver
//                   で後から書き戻す。
//
// 使い方: file_write [出力ディレクトリ]  （既定: カレントディレクトリ）
//
// ■ 異なる環境でも同じバイト列になるための約束
//   - 整数は幅が固定の型（std::uint16_t, std::uint32_t）で、
//     バイト順は常にビッグエンディアンと明示する（endian::big）。
//   - 構造体のメモリ表現（パディング・バイト順）をそのまま書き出さない。
//     1つずつ値を変換して書く。
//   - 文字列は UTF-8 のバイト列で、長さを前置する（終端文字は無い）。
//   - ファイルはバイナリモードで開く（file_writable_data_source が行う）。
//     改行コードの変換は起きない。
//
// ■ 木構造ファイル（tree.mbin）の配置  ［すべてビッグエンディアン］
//
//   位置  大きさ  内容
//   ----  ------  ---------------------------------------------
//    0      4     マジック "MTRE" (4D 54 52 45)
//    4      ...   ルートノード
//
//   ノード（深さ優先・行きがけ順で、子を親の直後に並べる）:
//     u16  名前のバイト数 N
//     N    名前（UTF-8）
//     u32  値
//     u16  子ノードの数 C
//     C×   子ノード（再帰）
//
//   本例のデータ: root(1) ─┬─ a(10) ── a1(11)
//                          └─ b(20)
//
//   00: 4D 54 52 45                     "MTRE"
//   04: 00 04 72 6F 6F 74               root の名前 (長さ4, "root")
//   0A: 00 00 00 01                     root の値 = 1
//   0E: 00 02                           root の子の数 = 2
//   10: 00 01 61 00 00 00 0A 00 01      a  (名前 "a",  値 10, 子 1)
//   19: 00 02 61 31 00 00 00 0B 00 00   a1 (名前 "a1", 値 11, 子 0)
//   23: 00 01 62 00 00 00 14 00 00      b  (名前 "b",  値 20, 子 0)
//   (全体 44 バイト)
//
// ■ 表ファイル（table.mbin）の配置  ［すべてビッグエンディアン］
//
//   位置  大きさ  内容
//   ----  ------  ---------------------------------------------
//    0      4     マジック "MTBL" (4D 54 42 4C)
//    4      4     u32 表の位置（ファイル先頭からのバイト数）
//    8      ...   文字列の領域: [u16 バイト数][UTF-8] を、続けて並べる
//    T      2     u16 文字列の数 M          ← T が、上の「表の位置」
//    T+2    4×M   u32 各文字列の位置（ファイル先頭から）
//
//   本例のデータ: "alpha", "beta", "gamma"
//
//   00: 4D 54 42 4C                     "MTBL"
//   04: 00 00 00 1C                     表の位置 = 0x1C (28)
//   08: 00 05 61 6C 70 68 61            "alpha"  （位置 8）
//   0F: 00 04 62 65 74 61               "beta"   （位置 15）
//   15: 00 05 67 61 6D 6D 61            "gamma"  （位置 21）
//   1C: 00 03                           文字列の数 = 3
//   1E: 00 00 00 08                     "alpha" の位置
//   22: 00 00 00 0F                     "beta"  の位置
//   26: 00 00 00 15                     "gamma" の位置
//   (全体 42 バイト)
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <vector>

import maboroutu.binary_convert;
import maboroutu.binary_layout_handle;
import maboroutu.file_data_source;
import maboroutu.memory_data_buffer;
import maboroutu.sequential_view;

namespace {

namespace mb = maboroutu;

// 木構造の1ノード。
struct node {
   std::string name;
   std::uint32_t value = 0;
   std::vector<node> children;
};

constexpr std::uint16_t max_u16 = std::numeric_limits<std::uint16_t>::max();

// マジック（ファイルの種類を示す、先頭4バイトの印）をバイト列として得る。
auto magic_bytes(std::string_view magic) -> std::span<std::byte const> {
   return std::as_bytes(std::span<char const>(magic));
}

// ---------------------------------------------------------------------
// 構造1: 木構造。sequential_view で、先頭から順に書く。
// ---------------------------------------------------------------------

// 1ノードを、現在の位置へ書く。子ノードは、再帰で直後に書く。
auto write_node(mb::writable_sequential_source auto &out,
                node const &n) -> bool {
   // 長さは u16 で表せる範囲に限る。
   if (n.name.size() > max_u16 || n.children.size() > max_u16) {
      return false;
   }
   if (!mb::write_value<mb::endian::big, std::uint16_t>(
           out, static_cast<std::uint16_t>(n.name.size()))) {
      return false;
   }
   if (!out.write(std::as_bytes(std::span<char const>(n.name)))) {
      return false;
   }
   if (!mb::write_value<mb::endian::big, std::uint32_t>(out, n.value)) {
      return false;
   }
   if (!mb::write_value<mb::endian::big, std::uint16_t>(
           out, static_cast<std::uint16_t>(n.children.size()))) {
      return false;
   }
   for (auto const &child : n.children) {
      if (!write_node(out, child)) {
         return false;
      }
   }
   return true;
}

auto write_tree(mb::memory_data_buffer &buffer, node const &root) -> bool {
   // sequential_view は、data_source の先頭から順に書き進める。
   mb::sequential_view out(buffer);
   if (!out.write(magic_bytes("MTRE"))) {
      return false;
   }
   return write_node(out, root);
}

// ---------------------------------------------------------------------
// 構造2: 文字列とオフセットの表。表の位置は、後から書き戻す。
// ---------------------------------------------------------------------

auto write_table(mb::memory_data_buffer &buffer,
                 std::vector<std::string> const &strings) -> bool {
   enum class handle_id : std::size_t {};
   // 4バイト・ビッグエンディアンの前方参照を扱う。
   mb::deferred_resolver<handle_id, 4, mb::endian::big, mb::memory_data_buffer>
       resolver;

   if (strings.size() > max_u16) {
      return false;
   }

   // 1. マジックを書き、「表の位置」の置き場所（4バイト）を予約する。
   //    表はまだ存在しないので、ハンドルだけ発行しておく。
   if (!buffer.append(magic_bytes("MTBL"))) {
      return false;
   }
   auto const table_handle = resolver.checkout();
   if (!resolver.defer_patch(buffer, table_handle)) {
      return false;
   }

   // 2. 文字列の領域を書く。各文字列の位置（オフセット）を覚えておく。
   std::vector<std::uint32_t> offsets;
   for (auto const &text : strings) {
      if (text.size() > max_u16) {
         return false;
      }
      // [u16 バイト数][UTF-8] の領域を確保して、その位置を記録する。
      auto const area = buffer.grow(sizeof(std::uint16_t) + text.size());
      if (!area) {
         return false;
      }
      offsets.push_back(static_cast<std::uint32_t>(area->offset));
      if (!mb::write_value<mb::endian::big, std::uint16_t>(
              buffer, area->offset, static_cast<std::uint16_t>(text.size()))) {
         return false;
      }
      if (!buffer.write(
              mb::region{.offset = area->offset + sizeof(std::uint16_t),
                         .size = text.size()},
              std::as_bytes(std::span<char const>(text)))) {
         return false;
      }
   }

   // 3. 表を書く。ここが「表の位置」であり、ハンドルへ確定する。
   auto const table = buffer.grow(sizeof(std::uint16_t) +
                                  offsets.size() * sizeof(std::uint32_t));
   if (!table) {
      return false;
   }
   resolver.resolve_handle(table_handle, table->offset);
   if (!mb::write_value<mb::endian::big, std::uint16_t>(
           buffer, table->offset, static_cast<std::uint16_t>(offsets.size()))) {
      return false;
   }
   for (std::size_t i = 0; i < offsets.size(); ++i) {
      auto const entry_offset =
          table->offset + sizeof(std::uint16_t) + i * sizeof(std::uint32_t);
      if (!mb::write_value<mb::endian::big, std::uint32_t>(buffer, entry_offset,
                                                           offsets[i])) {
         return false;
      }
   }

   // 4. 予約していた「表の位置」へ、確定した値を書き戻す。
   return static_cast<bool>(resolver.finalize());
}

// ---------------------------------------------------------------------
// メモリ上で組み立てたバイト列を、ファイルへ書き出す。
// ---------------------------------------------------------------------
auto save(mb::memory_data_buffer const &buffer,
          std::filesystem::path const &path) -> bool {
   auto const size = buffer.size();
   if (!size) {
      return false;
   }
   auto const bytes = buffer.read(mb::region{.offset = 0, .size = *size});
   if (!bytes) {
      return false;
   }
   // truncate: 既存の内容を捨てて、空のファイルとして開く。
   auto file =
       mb::file_writable_data_source::open(path, mb::file_open_mode::truncate);
   if (!file) {
      return false;
   }
   return static_cast<bool>(
       file->write(mb::region{.offset = 0, .size = *size},
                   std::span<std::byte const>(bytes->value.get(), *size)));
}

} // namespace

int main(int argc, char **argv) {
   std::filesystem::path const dir = (argc > 1) ? argv[1] : ".";

   // 構造1: 木構造。
   node const root{
       .name = "root",
       .value = 1,
       .children = {
           node{.name = "a",
                .value = 10,
                .children = {node{.name = "a1", .value = 11, .children = {}}}},
           node{.name = "b", .value = 20, .children = {}},
       }};
   mb::memory_data_buffer tree;
   if (!write_tree(tree, root) || !save(tree, dir / "tree.mbin")) {
      std::fprintf(stderr, "tree.mbin の書き込みに失敗しました\n");
      return 1;
   }

   // 構造2: 文字列とオフセットの表。
   mb::memory_data_buffer table;
   if (!write_table(table, {"alpha", "beta", "gamma"}) ||
       !save(table, dir / "table.mbin")) {
      std::fprintf(stderr, "table.mbin の書き込みに失敗しました\n");
      return 1;
   }

   std::printf("書き込みました: %s, %s\n", (dir / "tree.mbin").string().c_str(),
               (dir / "table.mbin").string().c_str());
   return 0;
}
