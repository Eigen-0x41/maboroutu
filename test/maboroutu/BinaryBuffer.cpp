#include <functional>
#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

#include "maboroutu/binaryBuffer.hpp"
#include "maboroutu/uniqueFile.hpp"
#include <bit>
#include <cstdint>
#include <iostream>
#include <print>

BOOST_AUTO_TEST_CASE(BinaryBuffer) {
  maboroutu::BinaryBuffer Buffer;
  maboroutu::UniqueFile UFile;

  BOOST_ASSERT(UFile.fopen_s("./test.bin", "rb+") == 0);

  UFile.fseek(0x80, maboroutu::UniqueFile::OffsetFlag::Begin);
  // UFile.fseek(0x0, maboroutu::UniqueFile::OffsetFlag::Begin);
  Buffer.load(UFile, 0x80);

  size_t const PosBase = 0x84;
  size_t Pos = PosBase;

  std::println(std::cout, "Pos:{:x}, value(big)   :{:x}, NextPos:{:x}", PosBase,
               Buffer.read<int32_t, std::endian::big>(std::ref(Pos)), Pos);
  Pos = PosBase;
  std::println(std::cout, "Pos:{:x}, value(little):{:x}, NextPos:{:x}", PosBase,
               Buffer.read<int32_t, std::endian::little>(std::ref(Pos)), Pos);
  Pos = PosBase;

  Buffer.write<int32_t, std::endian::big>(std::ref(Pos), 0x1021);
  std::println(std::cout, "write NextPos:{:x}", Pos);

  Buffer.store(UFile, true);
}
