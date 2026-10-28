# maboroutu

複数の層からなるアプリケーション設計（Binary File 〜 View の10層）で、複数の層から共通して必要とされる基盤を提供する、C++23/26 のモジュール方式のライブラリ。ファイルフォーマットに依存しない共通の構造（エラー型、データソース・バッファの抽象、数値のバイト列変換、レイアウト補助、補助コンテナ）のみを提供する。個別のフォーマットの解析やアプリケーション固有の処理は含まない。

- 開発中であり、バージョンは 0.0.x 系である。API は変更されうる。
- ライセンス: [MIT](LICENSE)

## 構成

| ターゲット | 内容 |
|---|---|
| `maboroutu::core` | エラー型・結果型、データソース／バッファ／シーケンシャルソースの抽象、数値変換、バイナリレイアウト補助 など |
| `maboroutu::extension` | `core` の具体実装・補助コンテナ（`memory_data_buffer`、`stream_data_source`、`slot_map` など）。`core` に依存する |

関連するリポジトリ:

- [maboroutu-tests](https://github.com/Eigen-0x41/maboroutu-tests): 本ライブラリのテスト（GoogleTest）と、CI から呼び出す検証ワークフロー。
- [maboroutu-appkit](https://github.com/Eigen-0x41/maboroutu-appkit): 実行ファイルの入口側の補助部品（コマンドライン引数の振り分け、`std::source_location` の整形）。かつては本ライブラリに含まれていた。

## 要件

- C++23 以上、CMake 3.28 以上、Ninja（C++ modules のため）。
- Clang 18 以上 + libc++ を主な動作環境とする。
- MSVC は 19.44 以上（`core.cppm` のバージョンガードによる）。CI で検証している。
- GCC は、現状ビルドできない。`core.cppm` には GCC 15 以上のガードがあるが、GCC 15・16 では C++ modules のモジュールファイル（BMI）の読み込みに失敗する事象（`failed to read compiled module cluster`）を確認している。
- 例外は無効（`-fno-exceptions`）でも利用できる（後述）。有効・無効は、本ライブラリと利用側で揃えること。

## 導入

CMake の `FetchContent` で取得する例（`maboroutu` を取得し、`maboroutu::extension` へリンクする）。

```cmake
cmake_minimum_required(VERSION 3.28)
project(app LANGUAGES CXX)

# モジュールを提供するライブラリと利用側で揃える（揃えないとコンパイルエラーになる）。
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(maboroutu
   GIT_REPOSITORY https://github.com/Eigen-0x41/maboroutu.git
   GIT_TAG        main)   # 再現性が必要な場合は、コミットまたはタグを指定する
FetchContent_MakeAvailable(maboroutu)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE maboroutu::extension)   # core のみなら maboroutu::core
set_target_properties(app PROPERTIES CXX_SCAN_FOR_MODULES ON)
```

- `extension` が不要な場合は `-DMABOROUTU_ENABLE_EXTENSION=OFF` とする。
- Clang + libc++ の場合は、`CMAKE_CXX_FLAGS` と `CMAKE_EXE_LINKER_FLAGS` に `-stdlib=libc++` を指定する。

## 使い方

モジュールを `import` して使う。範囲外の読み取りなどは、例外ではなく結果型（`std::expected` ベース）のエラーとして返る。

```cpp
#include <cstdint>
#include <cstdio>

import maboroutu.binary_convert;
import maboroutu.memory_data_buffer;

int main() {
   maboroutu::memory_data_buffer buffer;   // 空のバッファ

   auto const value =
       maboroutu::read_value<maboroutu::endian::big, std::uint32_t>(buffer, 0);
   if (!value) {
      auto const &error = value.error();
      std::printf("%.*s\n", static_cast<int>(error.message().size()),
                  error.message().data());   // 例: data_source: region is out of range
   }
}
```

動作する例は [`examples/`](examples) にある。`-DMABOROUTU_BUILD_EXAMPLES=ON` でビルドできる。

| 例 | 内容 |
|---|---|
| [`binary_roundtrip.cpp`](examples/binary_roundtrip.cpp) | 数値（整数・浮動小数点・配列）の書き込みと読み戻し |
| [`length_prefixed_patch.cpp`](examples/length_prefixed_patch.cpp) | 本体を書いた後で、先頭の長さフィールドを書き戻す |
| [`error_handling.cpp`](examples/error_handling.cpp) | 結果型・エラーコード・エラーメッセージ |

### エラー処理の方針

- **使用方法の誤り**（事前条件の違反）は、Debug ビルドで `assert` により検出する。Release ビルドでは検出せず、未定義動作となる。
- コンパイル時に検出できる誤りは、`static_assert` と concept で防ぐ。
- 独自の API は、原則として `std::expected`（`result<T, Code>`）で、復帰可能な失敗を返す。外部入力に由来する値の誤りも、これに含まれる。
- STL に近い使用感のクラス（`slot_map` など）は、STL と同様に、呼び出し側が事前に確認する関数（`contains()` など）で誤りを防ぐことを前提とする。
- 例外は、復帰できない致命的なエラーに限って送出する。例外が無効な場合は、`std::abort` または `std::terminate` で終了する（次節）。

### 例外が無効な場合

`-fno-exceptions` では、復帰できないエラーは、例外の代わりにプログラムの終了（既定は `std::abort`）になる。

| マクロ | 内容 |
|---|---|
| `MABOROUTU_FATAL_TO_TERMINATE` | 定義すると、`std::abort` の代わりに `std::terminate` で終了する |
| `MABOROUTU_FATAL_DIAGNOSTICS` | `0`/`1`。終了前に、固定の診断文字列（`enter_fatal` にコンパイル時定数のメッセージを渡した場合は、そのメッセージを含む）を標準エラーへ出力する。未定義の場合は、`NDEBUG` 未定義のとき `1`、定義済みのとき `0` |

## モジュール一覧

| モジュール | ターゲット | 内容 |
|---|---|---|
| `maboroutu.core` | core | 共通の型・定数・補助 |
| `maboroutu.error` | core | `error<Code, Detail>`、`result<T, Code>`、`make_unexpected`、`enter_fatal` |
| `maboroutu.data_source` | core | オフセット・サイズ指定で読み書きするデータソースの抽象（concept と型消去ラッパー） |
| `maboroutu.data_buffer` | core | 追記・領域予約・ビュー取得を持つバッファの抽象、`segmented_span` |
| `maboroutu.sequential_source` | core | 前方向のみのシーケンシャルアクセスの抽象 |
| `maboroutu.sequential_view` | core | `data_source` の上に重ねる、累積バイト数のみを持つ非所有ビュー |
| `maboroutu.binary_convert` | core | バイト列と数値型との相互変換（エンディアン指定） |
| `maboroutu.binary_layout` | core | 任意バイト幅（1〜8）の符号なし整数の読み書き、後から値を書き戻す `position_patch` |
| `maboroutu.io_pattern` | core | reader / writer / mapper の concept とエラー型（実装は含まない） |
| `maboroutu.extension_store` | core | enum と型の対応付けによる拡張機構 |
| `maboroutu.memory_data_buffer` | extension | メモリ上のバイト列を保持する `data_buffer` の実装 |
| `maboroutu.stream_data_source` | extension | `std::istream` / `std::iostream` を `data_source` として扱う実装 |
| `maboroutu.binary_layout_handle` | extension | ハンドルによる遅延解決（`deferred_resolver`） |
| `maboroutu.slot_map` | extension | 安定したハンドル（インデックス）を持つフリーリスト型のコンテナ |
| `maboroutu.keyed_slot_map` | extension | 任意のキーから `slot_map` の要素へ振り分けるラッパー |

## テストと CI

- テストは [maboroutu-tests](https://github.com/Eigen-0x41/maboroutu-tests) にある。
- 読み取った値やエラーを print debug するための整形は、[maboroutu-appkit](https://github.com/Eigen-0x41/maboroutu-appkit) の `maboroutu.result_format` にある。本体は、標準出力・標準エラー出力へ書き込まない（`enter_fatal` の診断表示を除く）。
- GitHub Actions（[`.github/workflows/ci.yml`](.github/workflows/ci.yml)）で、Clang・MSVC・サニタイザ（ASan + UBSan）のテスト、clang-tidy による静的解析、`examples/` のビルドを実行する。MSVC のコード解析（`/analyze`、C++ Core Check）と Clang のライフタイム解析は、手動実行（`analysis` を有効にした `workflow_dispatch`）でのみ実行する。

## LLM の利用について

本ライブラリの開発には、LLM（Anthropic の Claude）を利用している。LLM が出力した内容は、作者が確認し、作者自身が適用・コミットしている。

- **2026-10-15 以降の改善**（リポジトリの履歴の `validate before allocate` から `package metadata` まで）は、LLM による評価をもとに、LLM が作成したパッチを作者が確認・適用する形で行った。対象は次のとおり。
  - 確保前の検証（`memory_data_buffer`・`stream_data_source`）
  - 既知の不具合の修正（`binary_layout`・`data_buffer`・`memory_data_buffer`）
  - CI（GitHub Actions）と clang-tidy の設定
  - エラーメッセージ（`error_message`・`basic_error::message()`）と `enter_fatal` の診断表示
  - 綴りの誤りの修正
  - `console`・`source_location_format` の `maboroutu-appkit` への分離
  - パッケージメタデータ（CMake の互換性判定など）
  - 本 README と `examples/`
  - 上記に対応する `maboroutu-tests` のテストと、仕様書の改訂
- **それ以前**は、設計の検討、仕様書の作成・改訂、`maboroutu-tests` のテストの作成、コンパイラごとの不具合の調査に利用した。ライブラリ本体のコードのうち LLM が生成した範囲を示す記録は、残していない。
- 設計の判断（採用する方針、公開する範囲、リポジトリの分割など）は、作者が行っている。
