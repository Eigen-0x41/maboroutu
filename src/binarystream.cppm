module;
#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <istream>
#include <ratio>
#include <vector>

export module maboroutu.binarystream;
export import :basic_ibinarystream;
export import :basic_obinarystream;
export import :basic_iobinarystream;

export namespace maboroutu {

using ibinarystream = basic_ibinarystream<char>;
using wibinarystream = basic_ibinarystream<wchar_t>;

using obinarystream = basic_obinarystream<char>;
using wobinarystream = basic_obinarystream<wchar_t>;

using iobinarystream = basic_iobinarystream<char>;
using wiobinarystream = basic_iobinarystream<wchar_t>;

} // namespace maboroutu
