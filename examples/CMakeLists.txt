cmake_minimum_required(VERSION 3.28)

# maboroutu の利用例。リポジトリのルートから
#   cmake -S . -B build -G Ninja -DMABOROUTU_BUILD_EXAMPLES=ON
# で有効にする。
project(maboroutu_examples LANGUAGES CXX)

# モジュールを提供するライブラリと利用側で、CMAKE_CXX_EXTENSIONSを揃える
# （揃えないと、モジュールファイルの構成不一致でコンパイルエラーになる）。
set(CMAKE_CXX_EXTENSIONS OFF)

function(maboroutu_add_example name)
   add_executable(${name} ${name}.cpp)
   target_link_libraries(${name} PRIVATE maboroutu::extension)
   set_target_properties(${name} PROPERTIES CXX_SCAN_FOR_MODULES ON)
endfunction()

maboroutu_add_example(binary_roundtrip)
maboroutu_add_example(length_prefixed_patch)
maboroutu_add_example(error_handling)
