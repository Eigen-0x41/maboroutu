// ファイルから、2種類のバイナリ構造を読み込む例（書き込みは file_write.cpp）。
//
//   read_tree  ... 木構造（入れ子のノード）を、先頭から順に読む。
//                  sequential_view が、読み取り位置を自動で進める。
//   read_table ... オフセットが指す位置へ、直接読みに行く。
//                  data_source は、位置を指定して読む（ランダムアクセス）。
//
// 使い方: file_read [入力ディレクトリ]  （既定: カレントディレクトリ）
//         先に file_write で、tree.mbin と table.mbin を作っておく。
//
// ■ 異なる環境でも同じ結果になるための約束（file_write.cpp と同じ）
//   - 整数は幅が固定の型で、バイト順はビッグエンディアンと明示して読む。
//   - 構造体のメモリ表現を、そのまま読み込まない。1つずつ値を読む。
//   - 文字列は UTF-8 のバイト列で、長さが前置されている。
//   - ファイルはバイナリモードで開く（file_data_source が行う）。
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
//   例（root(1) ─┬─ a(10) ── a1(11)、└─ b(20)）の、先頭からのバイト列:
//   4D 54 52 45 | 00 04 72 6F 6F 74 00 00 00 01 00 02 |
//   00 01 61 00 00 00 0A 00 01 | 00 02 61 31 00 00 00 0B 00 00 |
//   00 01 62 00 00 00 14 00 00                      (全体 44 バイト)
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
//   読み方: 4 から「表の位置」T を読む → T から M を読む →
//           T+2+4×i から i 番目の文字列の位置 P を読む →
//           P から [u16 バイト数 L][L バイト] を読む。
//
//   例（"alpha", "beta", "gamma"）:
//   00: 4D 54 42 4C | 00 00 00 1C                 マジック, 表の位置 = 28
//   08: 00 05 61 6C 70 68 61                      "alpha"（位置 8）
//   0F: 00 04 62 65 74 61                         "beta" （位置 15）
//   15: 00 05 67 61 6D 6D 61                      "gamma"（位置 21）
//   1C: 00 03 | 00 00 00 08 | 00 00 00 0F | 00 00 00 15   M = 3, 各位置
//   (全体 42 バイト)
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

import maboroutu.binary_convert;
import maboroutu.file_data_source;
import maboroutu.sequential_view;

namespace {

namespace mb = maboroutu;

// 木構造の1ノード。
struct node {
   std::string name;
   std::uint32_t value = 0;
   std::vector<node> children;
};

// 入れ子の深さの上限。壊れた（または悪意のある）ファイルが、
// 深い再帰を起こして、スタックを使い尽くすことを防ぐ。
constexpr std::size_t max_depth = 32;

// 先頭のマジックを確認する。読み込めない、または一致しない場合は false。
auto has_magic(std::span<std::byte const> actual,
               std::string_view expected) -> bool {
   auto const want = std::as_bytes(std::span<char const>(expected));
   return actual.size() == want.size() &&
          std::equal(actual.begin(), actual.end(), want.begin());
}

// ---------------------------------------------------------------------
// 構造1: 木構造。sequential_view で、先頭から順に読む。
// ---------------------------------------------------------------------

// 1ノードを、現在の位置から読む。子ノードは、再帰で直後から読む。
auto read_node(mb::sequential_source auto &in,
               std::size_t depth) -> mb::sequential_source_result<node> {
   if (depth > max_depth) {
      return mb::make_unexpected(
          mb::errc::sequential_source::invalid_member_variable);
   }

   node result;

   // 名前: [u16 バイト数][UTF-8]。バイト数は u16 なので、最大 65535。
   auto name_length = mb::read_value<mb::endian::big, std::uint16_t>(in);
   if (!name_length) {
      return std::unexpected(std::move(name_length).error());
   }
   result.name.resize(*name_length);
   if (auto read = in.read_into(*name_length,
                                std::as_writable_bytes(std::span(result.name)));
       !read) {
      return std::unexpected(std::move(read).error());
   }

   // 値: u32。
   auto value = mb::read_value<mb::endian::big, std::uint32_t>(in);
   if (!value) {
      return std::unexpected(std::move(value).error());
   }
   result.value = *value;

   // 子ノード: 数を読んで、その数だけ再帰で読む。
   // 数をそのまま reserve しない（壊れたファイルで、巨大な確保をしないため）。
   // ファイルが途中で終わっていれば、読み取りが範囲外のエラーで止まる。
   auto child_count = mb::read_value<mb::endian::big, std::uint16_t>(in);
   if (!child_count) {
      return std::unexpected(std::move(child_count).error());
   }
   for (std::uint16_t i = 0; i < *child_count; ++i) {
      auto child = read_node(in, depth + 1);
      if (!child) {
         return std::unexpected(std::move(child).error());
      }
      result.children.push_back(std::move(*child));
   }
   return result;
}

auto read_tree(mb::data_source auto &src)
    -> mb::sequential_source_result<node> {
   // sequential_view は、data_source の先頭から順に読み進める。
   // 読むたびに、位置（count()）が、読んだバイト数だけ進む。
   mb::sequential_view in(src);

   std::array<std::byte, 4> magic{};
   if (auto read = in.read_into(magic.size(), magic); !read) {
      return std::unexpected(std::move(read).error());
   }
   if (!has_magic(magic, "MTRE")) {
      return mb::make_unexpected(
          mb::errc::sequential_source::invalid_member_variable);
   }
   return read_node(in, 0);
}

// ---------------------------------------------------------------------
// 構造2: オフセットで指された位置を、直接読む。
// ---------------------------------------------------------------------

auto read_table(mb::data_source auto &src)
    -> mb::data_source_result<std::vector<std::string>> {
   // 先頭のマジック。位置を指定して読む（region = {位置, バイト数}）。
   std::array<std::byte, 4> magic{};
   if (auto read =
           src.read_into(mb::region{.offset = 0, .size = magic.size()}, magic);
       !read) {
      return std::unexpected(std::move(read).error());
   }
   if (!has_magic(magic, "MTBL")) {
      return mb::make_unexpected(mb::errc::data_source::invalid_operation);
   }

   // 位置 4 に、表の位置が書かれている。
   auto table_offset = mb::read_value<mb::endian::big, std::uint32_t>(src, 4);
   if (!table_offset) {
      return std::unexpected(std::move(table_offset).error());
   }

   // 表の先頭: 文字列の数。
   auto count =
       mb::read_value<mb::endian::big, std::uint16_t>(src, *table_offset);
   if (!count) {
      return std::unexpected(std::move(count).error());
   }

   std::vector<std::string> strings;
   for (std::size_t i = 0; i < *count; ++i) {
      // i 番目の文字列の位置。範囲外の位置は、読み取りがエラーを返す。
      auto const entry =
          *table_offset + sizeof(std::uint16_t) + i * sizeof(std::uint32_t);
      auto text_offset =
          mb::read_value<mb::endian::big, std::uint32_t>(src, entry);
      if (!text_offset) {
         return std::unexpected(std::move(text_offset).error());
      }

      // その位置から、[u16 バイト数][UTF-8] を読む。
      auto length =
          mb::read_value<mb::endian::big, std::uint16_t>(src, *text_offset);
      if (!length) {
         return std::unexpected(std::move(length).error());
      }
      std::string text(*length, '\0');
      if (auto read = src.read_into(
              mb::region{.offset = *text_offset + sizeof(std::uint16_t),
                         .size = text.size()},
              std::as_writable_bytes(std::span(text)));
          !read) {
         return std::unexpected(std::move(read).error());
      }
      strings.push_back(std::move(text));
   }
   return strings;
}

void print_node(node const &n, int indent) {
   std::printf("%*s%s = %u\n", indent * 2, "", n.name.c_str(),
               static_cast<unsigned>(n.value));
   for (auto const &child : n.children) {
      print_node(child, indent + 1);
   }
}

} // namespace

int main(int argc, char **argv) {
   std::filesystem::path const dir = (argc > 1) ? argv[1] : ".";

   // 構造1: 木構造。
   auto tree_file = mb::file_data_source::open(dir / "tree.mbin");
   if (!tree_file) {
      std::fprintf(stderr, "tree.mbin を開けません\n");
      return 1;
   }
   auto const root = read_tree(*tree_file);
   if (!root) {
      std::fprintf(stderr, "tree.mbin の読み込みに失敗しました\n");
      return 1;
   }
   print_node(*root, 0);

   // 構造2: オフセットの表。
   auto table_file = mb::file_data_source::open(dir / "table.mbin");
   if (!table_file) {
      std::fprintf(stderr, "table.mbin を開けません\n");
      return 1;
   }
   auto const strings = read_table(*table_file);
   if (!strings) {
      std::fprintf(stderr, "table.mbin の読み込みに失敗しました\n");
      return 1;
   }
   for (std::size_t i = 0; i < strings->size(); ++i) {
      std::printf("[%zu] %s\n", i, (*strings)[i].c_str());
   }
   return 0;
}
