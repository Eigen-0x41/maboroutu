#define BOOST_TEST_MAIN

#include "maboroutu/slotMap.hpp"
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <print>
#include <stdexcept>

BOOST_AUTO_TEST_CASE(SlotMap) {
  maboroutu::SlotMap<int> Test;


  auto I1 = Test.insert(0);
  auto I2 = Test.insert(1);
  auto I3 = Test.insert(2);
  auto I4 = Test.insert(3);
  std::println(std::cout, "i:{} v:{}", Test.__DEBUG_F__GET_KEY_INDEX_(I1), Test.at(I1));
  std::println(std::cout, "i:{} v:{}", Test.__DEBUG_F__GET_KEY_INDEX_(I2), Test.at(I2));
  std::println(std::cout, "i:{} v:{}", Test.__DEBUG_F__GET_KEY_INDEX_(I3), Test.at(I3));
  std::println(std::cout, "i:{} v:{}", Test.__DEBUG_F__GET_KEY_INDEX_(I4), Test.at(I4));
  BOOST_CHECK_EQUAL(Test.at(I1), 0);
  BOOST_CHECK_EQUAL(Test.at(I2), 1);
  BOOST_CHECK_EQUAL(Test.at(I3), 2);
  BOOST_CHECK_EQUAL(Test.at(I4), 3);
  std::println(std::cout, "size: {}, capacity: {}", Test.size(),
               Test.freeSize());

  Test.erase(I2);
  Test.erase(I4);
  I2 = Test.emplace(14);
  I4 = Test.emplace(15);

  std::println(std::cout, "i:{} v:{}", Test.__DEBUG_F__GET_KEY_INDEX_(I1), Test.at(I1));
  std::println(std::cout, "i:{} v:{}", Test.__DEBUG_F__GET_KEY_INDEX_(I2), Test.at(I2));
  std::println(std::cout, "i:{} v:{}", Test.__DEBUG_F__GET_KEY_INDEX_(I3), Test.at(I3));
  std::println(std::cout, "i:{} v:{}", Test.__DEBUG_F__GET_KEY_INDEX_(I4), Test.at(I4));
  BOOST_CHECK_EQUAL(Test.at(I1), 0);
  BOOST_CHECK_EQUAL(Test.at(I2), 14);
  BOOST_CHECK_EQUAL(Test.at(I3), 2);
  BOOST_CHECK_EQUAL(Test.at(I4), 15);
  std::println(std::cout, "size: {}, capacity: {}", Test.size(),
               Test.freeSize());

  Test.erase(I1);
  Test.erase(I2);
  Test.erase(I4);
  I1 = Test.emplace(26);
  // I2 = Test.emplaceBack(27);
  I4 = Test.insert(28);

  std::println(std::cout, "i:{} v:{}", Test.__DEBUG_F__GET_KEY_INDEX_(I1), Test.at(I1));
  // std::println(std::cout, "i:{} v:{}", Test.__DEBUG_F__GET_KEY_INDEX_(I2), Test.at(I2));
  std::println(std::cout, "i:{} v:{}", Test.__DEBUG_F__GET_KEY_INDEX_(I3), Test.at(I3));
  std::println(std::cout, "i:{} v:{}", Test.__DEBUG_F__GET_KEY_INDEX_(I4), Test.at(I4));
  BOOST_CHECK_EQUAL(Test.at(I1), 26);
  // BOOST_CHECK_EQUAL(Test.at(I2), 27);
  BOOST_CHECK_EQUAL(Test.at(I3), 2);
  BOOST_CHECK_EQUAL(Test.at(I4), 28);
  std::println(std::cout, "size: {}, capacity: {}", Test.size(),
               Test.freeSize());

  // BOOST_CHECK_THROW(Test.at(4), std::out_of_range);
}
