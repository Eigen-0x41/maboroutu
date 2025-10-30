#include <boost/test/tools/old/interface.hpp>
#include <exception>
#define BOOST_TEST_MAIN

#include "maboroutu/slot_map.hpp"
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <print>
#include <stdexcept>

BOOST_AUTO_TEST_CASE(SlotMap) {
  using slot_map_type = maboroutu::array_slot_map<int,4>;
  slot_map_type test;

  slot_map_type::key_type i1;
  slot_map_type::key_type i2;
  slot_map_type::key_type i3;
  slot_map_type::key_type i4;
  BOOST_CHECK_NO_THROW(i1 = test.insert(0));
  BOOST_CHECK_NO_THROW(i2 = test.insert(1));
  BOOST_CHECK_NO_THROW(i3 = test.insert(2));
  BOOST_CHECK_NO_THROW(i4 = test.insert(3));
  std::println(std::cout, "i:{} v:{}", test.debug_f_get_key_index(i1),
               test.at(i1));
  std::println(std::cout, "i:{} v:{}", test.debug_f_get_key_index(i2),
               test.at(i2));
  std::println(std::cout, "i:{} v:{}", test.debug_f_get_key_index(i3),
               test.at(i3));
  std::println(std::cout, "i:{} v:{}", test.debug_f_get_key_index(i4),
               test.at(i4));
  BOOST_CHECK_EQUAL(test.at(i1), 0);
  BOOST_CHECK_EQUAL(test.at(i2), 1);
  BOOST_CHECK_EQUAL(test.at(i3), 2);
  BOOST_CHECK_EQUAL(test.at(i4), 3);
  std::println(std::cout, "size: {}, capacity: {}", test.size(),
               test.free_size());

  test.erase(i2);
  test.erase(i4);
  BOOST_CHECK_NO_THROW(i2 = test.emplace(14));
  BOOST_CHECK_NO_THROW(i4 = test.emplace(15));

  std::println(std::cout, "i:{} v:{}", test.debug_f_get_key_index(i1),
               test.at(i1));
  std::println(std::cout, "i:{} v:{}", test.debug_f_get_key_index(i2),
               test.at(i2));
  std::println(std::cout, "i:{} v:{}", test.debug_f_get_key_index(i3),
               test.at(i3));
  std::println(std::cout, "i:{} v:{}", test.debug_f_get_key_index(i4),
               test.at(i4));
  BOOST_CHECK_EQUAL(test.at(i1), 0);
  BOOST_CHECK_EQUAL(test.at(i2), 14);
  BOOST_CHECK_EQUAL(test.at(i3), 2);
  BOOST_CHECK_EQUAL(test.at(i4), 15);
  std::println(std::cout, "size: {}, capacity: {}", test.size(),
               test.free_size());

  test.erase(i1);
  test.erase(i2);
  test.erase(i4);
  BOOST_CHECK_NO_THROW(i1 = test.emplace(26));
  // BOOST_CHECK_NO_THROW(I2 = Test.emplaceBack(27));
  BOOST_CHECK_NO_THROW(i4 = test.insert(28));

  std::println(std::cout, "i:{} v:{}", test.debug_f_get_key_index(i1),
               test.at(i1));
  // std::println(std::cout, "i:{} v:{}", Test.debug_f_get_key_index(I2),
  // Test.at(I2));
  std::println(std::cout, "i:{} v:{}", test.debug_f_get_key_index(i3),
               test.at(i3));
  std::println(std::cout, "i:{} v:{}", test.debug_f_get_key_index(i4),
               test.at(i4));
  BOOST_CHECK_EQUAL(test.at(i1), 26);
  // BOOST_CHECK_EQUAL(Test.at(I2), 27);
  BOOST_CHECK_EQUAL(test.at(i3), 2);
  BOOST_CHECK_EQUAL(test.at(i4), 28);
  std::println(std::cout, "size: {}, capacity: {}", test.size(),
               test.free_size());

  i2 = test.emplace(0);
  BOOST_CHECK_THROW(test.emplace(39), std::out_of_range);
}
