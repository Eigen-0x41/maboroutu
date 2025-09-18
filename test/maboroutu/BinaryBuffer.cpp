#include "maboroutu/binaryBuffer.hpp"
#include "maboroutu/uniqueFile.hpp"
#include <bit>
#define BOOST_TEST_MAIN

#include <boost/test/included/unit_test.hpp>
#include <cstdint>
#include <iostream>
#include <print>

BOOST_AUTO_TEST_CASE(BinaryBuffer) {
  maboroutu::BinaryBuffer Buffer;
  maboroutu::UniqueFile UFile;

  if (UFile.fopen_s("./test.bin", "rb+")) {
    std::println(std::cout, "Filed to load file!");
    return;
  }

  UFile.fseek(0x80, maboroutu::UniqueFile::OffsetFlag::Begin);
  // UFile.fseek(0x0, maboroutu::UniqueFile::OffsetFlag::Begin);
  Buffer.load(UFile, 0x80);

  std::println(std::cout, "value(big)   : {:x}",
               Buffer.read<int32_t, std::endian::big>(0x84));
  std::println(std::cout, "value(little): {:x}",
               Buffer.read<int32_t, std::endian::little>(0x84));

  Buffer.write<int32_t, std::endian::big>(0x84, 0x1021);
  Buffer.store(UFile, true);
}
