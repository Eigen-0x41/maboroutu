#include <boost/test/tools/old/interface.hpp>
#include <exception>
#define BOOST_TEST_MAIN

#include "maboroutu/slot_map.hpp"
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <print>
#include <stdexcept>

struct slot_tag0 {};
struct slot_tag1 {};

BOOST_AUTO_TEST_CASE(SlotMap) {
  std::println("test0");
  using slot_map_type = maboroutu::array_slot_map<slot_tag0, int, 4>;
  slot_map_type test0;

  slot_map_type::key_type i01;
  slot_map_type::key_type i02;
  slot_map_type::key_type i03;
  slot_map_type::key_type i04;
  BOOST_CHECK_NO_THROW(i01 = test0.insert(0));
  BOOST_CHECK_NO_THROW(i02 = test0.insert(1));
  BOOST_CHECK_NO_THROW(i03 = test0.insert(2));
  BOOST_CHECK_NO_THROW(i04 = test0.insert(3));
  std::println(std::cout, "i:{} v:{}", test0.debug_f_get_key_index(i01),
               test0.at(i01));
  std::println(std::cout, "i:{} v:{}", test0.debug_f_get_key_index(i02),
               test0.at(i02));
  std::println(std::cout, "i:{} v:{}", test0.debug_f_get_key_index(i03),
               test0.at(i03));
  std::println(std::cout, "i:{} v:{}", test0.debug_f_get_key_index(i04),
               test0.at(i04));
  BOOST_CHECK_EQUAL(test0.at(i01), 0);
  BOOST_CHECK_EQUAL(test0.at(i02), 1);
  BOOST_CHECK_EQUAL(test0.at(i03), 2);
  BOOST_CHECK_EQUAL(test0.at(i04), 3);
  std::println(std::cout, "size: {}, capacity: {}", test0.size(),
               test0.free_size());

  test0.erase(i02);
  test0.erase(i04);
  BOOST_CHECK_NO_THROW(i02 = test0.emplace(14));
  BOOST_CHECK_NO_THROW(i04 = test0.emplace(15));

  std::println(std::cout, "i:{} v:{}", test0.debug_f_get_key_index(i01),
               test0.at(i01));
  std::println(std::cout, "i:{} v:{}", test0.debug_f_get_key_index(i02),
               test0.at(i02));
  std::println(std::cout, "i:{} v:{}", test0.debug_f_get_key_index(i03),
               test0.at(i03));
  std::println(std::cout, "i:{} v:{}", test0.debug_f_get_key_index(i04),
               test0.at(i04));
  BOOST_CHECK_EQUAL(test0.at(i01), 0);
  BOOST_CHECK_EQUAL(test0.at(i02), 14);
  BOOST_CHECK_EQUAL(test0.at(i03), 2);
  BOOST_CHECK_EQUAL(test0.at(i04), 15);
  std::println(std::cout, "size: {}, capacity: {}", test0.size(),
               test0.free_size());

  test0.erase(i01);
  test0.erase(i02);
  test0.erase(i04);
  BOOST_CHECK_NO_THROW(i01 = test0.emplace(26));
  // BOOST_CHECK_NO_THROW(I2 = Test.emplaceBack(27));
  BOOST_CHECK_NO_THROW(i04 = test0.insert(28));

  std::println(std::cout, "i:{} v:{}", test0.debug_f_get_key_index(i01),
               test0.at(i01));
  // std::println(std::cout, "i:{} v:{}", Test.debug_f_get_key_index(I2),
  // Test.at(I2));
  std::println(std::cout, "i:{} v:{}", test0.debug_f_get_key_index(i03),
               test0.at(i03));
  std::println(std::cout, "i:{} v:{}", test0.debug_f_get_key_index(i04),
               test0.at(i04));
  BOOST_CHECK_EQUAL(test0.at(i01), 26);
  // BOOST_CHECK_EQUAL(Test.at(I2), 27);
  BOOST_CHECK_EQUAL(test0.at(i03), 2);
  BOOST_CHECK_EQUAL(test0.at(i04), 28);
  std::println(std::cout, "size: {}, capacity: {}", test0.size(),
               test0.free_size());

  i02 = test0.emplace(0);
  BOOST_CHECK_THROW(test0.emplace(39), std::out_of_range);

  std::println("test1");
  using slot_map_type1 = maboroutu::array_slot_map<slot_tag1, int, 4>;
  slot_map_type1 test1;

  slot_map_type1::key_type i11;
  slot_map_type1::key_type i12;
  slot_map_type1::key_type i13;
  slot_map_type1::key_type i14;
  BOOST_CHECK_NO_THROW(i11 = test1.insert(0));
  BOOST_CHECK_NO_THROW(i12 = test1.insert(1));
  BOOST_CHECK_NO_THROW(i13 = test1.insert(2));
  BOOST_CHECK_NO_THROW(i14 = test1.insert(3));
  std::println(std::cout, "i:{} v:{}", test1.debug_f_get_key_index(i11),
               test1.at(i11));
  std::println(std::cout, "i:{} v:{}", test1.debug_f_get_key_index(i12),
               test1.at(i12));
  std::println(std::cout, "i:{} v:{}", test1.debug_f_get_key_index(i13),
               test1.at(i13));
  std::println(std::cout, "i:{} v:{}", test1.debug_f_get_key_index(i14),
               test1.at(i14));
  BOOST_CHECK_EQUAL(test1.at(i11), 0);
  BOOST_CHECK_EQUAL(test1.at(i12), 1);
  BOOST_CHECK_EQUAL(test1.at(i13), 2);
  BOOST_CHECK_EQUAL(test1.at(i14), 3);
  std::println(std::cout, "size: {}, capacity: {}", test1.size(),
               test1.free_size());

  test1.erase(i12);
  test1.erase(i14);
  BOOST_CHECK_NO_THROW(i12 = test1.emplace(14));
  BOOST_CHECK_NO_THROW(i14 = test1.emplace(15));

  std::println(std::cout, "i:{} v:{}", test1.debug_f_get_key_index(i11),
               test1.at(i11));
  std::println(std::cout, "i:{} v:{}", test1.debug_f_get_key_index(i12),
               test1.at(i12));
  std::println(std::cout, "i:{} v:{}", test1.debug_f_get_key_index(i13),
               test1.at(i13));
  std::println(std::cout, "i:{} v:{}", test1.debug_f_get_key_index(i14),
               test1.at(i14));
  BOOST_CHECK_EQUAL(test1.at(i11), 0);
  BOOST_CHECK_EQUAL(test1.at(i12), 14);
  BOOST_CHECK_EQUAL(test1.at(i13), 2);
  BOOST_CHECK_EQUAL(test1.at(i14), 15);
  std::println(std::cout, "size: {}, capacity: {}", test1.size(),
               test1.free_size());

  test1.erase(i11);
  test1.erase(i12);
  test1.erase(i14);
  BOOST_CHECK_NO_THROW(i11 = test1.emplace(26));
  // BOOST_CHECK_NO_THROW(I2 = Test.emplaceBack(27));
  BOOST_CHECK_NO_THROW(i14 = test1.insert(28));

  std::println(std::cout, "i:{} v:{}", test1.debug_f_get_key_index(i11),
               test1.at(i11));
  // std::println(std::cout, "i:{} v:{}", Test.debug_f_get_key_index(I2),
  // Test.at(I2));
  std::println(std::cout, "i:{} v:{}", test1.debug_f_get_key_index(i13),
               test1.at(i13));
  std::println(std::cout, "i:{} v:{}", test1.debug_f_get_key_index(i14),
               test1.at(i14));
  BOOST_CHECK_EQUAL(test1.at(i11), 26);
  // BOOST_CHECK_EQUAL(Test.at(I2), 27);
  BOOST_CHECK_EQUAL(test1.at(i13), 2);
  BOOST_CHECK_EQUAL(test1.at(i14), 28);
  std::println(std::cout, "size: {}, capacity: {}", test1.size(),
               test1.free_size());

  i12 = test1.emplace(0);
  BOOST_CHECK_THROW(test1.emplace(39), std::out_of_range);

  // i01 = test1.emplace(2); // cannot compiling.
}
