#define BOOST_TEST_MAIN

#include "maboroutu/slotMap.hpp"
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <print>
#include <stdexcept>

BOOST_AUTO_TEST_CASE(SlotMap) {
  maboroutu::SlotMap<int> Test;

  auto I1 = *Test.emplaceExpected(0);
  auto I2 = *Test.emplaceExpected(1);
  auto I3 = *Test.emplaceExpected(2);
  auto I4 = *Test.emplaceExpected(3);

  std::println(std::cout, "i:{} v:{}", I1, Test.at(I1));
  std::println(std::cout, "i:{} v:{}", I2, Test.at(I2));
  std::println(std::cout, "i:{} v:{}", I3, Test.at(I3));
  std::println(std::cout, "i:{} v:{}", I4, Test.at(I4));
  BOOST_CHECK_EQUAL(Test.at(I1), 0);
  BOOST_CHECK_EQUAL(Test.at(I2), 1);
  BOOST_CHECK_EQUAL(Test.at(I3), 2);
  BOOST_CHECK_EQUAL(Test.at(I4), 3);

  Test.erase(I2);
  Test.erase(I4);
  I2 = *Test.emplaceExpected(14);
  I4 = *Test.emplaceExpected(15);

  std::println(std::cout, "i:{} v:{}", I1, Test.at(I1));
  std::println(std::cout, "i:{} v:{}", I2, Test.at(I2));
  std::println(std::cout, "i:{} v:{}", I3, Test.at(I3));
  std::println(std::cout, "i:{} v:{}", I4, Test.at(I4));
  BOOST_CHECK_EQUAL(Test.at(I1), 0);
  BOOST_CHECK_EQUAL(Test.at(I2), 14);
  BOOST_CHECK_EQUAL(Test.at(I3), 2);
  BOOST_CHECK_EQUAL(Test.at(I4), 15);

  Test.erase(I1);
  Test.erase(I2);
  Test.erase(I4);
  I1 = *Test.emplaceExpected(26);
  I2 = *Test.emplaceExpected(27);
  I4 = *Test.emplaceExpected(28);

  std::println(std::cout, "i:{} v:{}", I1, Test.at(I1));
  std::println(std::cout, "i:{} v:{}", I2, Test.at(I2));
  std::println(std::cout, "i:{} v:{}", I3, Test.at(I3));
  std::println(std::cout, "i:{} v:{}", I4, Test.at(I4));
  BOOST_CHECK_EQUAL(Test.at(I1), 26);
  BOOST_CHECK_EQUAL(Test.at(I2), 27);
  BOOST_CHECK_EQUAL(Test.at(I3), 2);
  BOOST_CHECK_EQUAL(Test.at(I4), 28);

  BOOST_CHECK_THROW(Test.at(4), std::out_of_range);
}
