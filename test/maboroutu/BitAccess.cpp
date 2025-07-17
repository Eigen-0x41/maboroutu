#define BOOST_TEST_MAIN

#include "maboroutu/bitAccess.hpp"
#include <boost/test/included/unit_test.hpp>
#include <cstdint>
#include <iostream>
#include <print>

BOOST_AUTO_TEST_CASE(BitAccess) {
  using field_type = uint16_t;

  using bitAccess_type = maboroutu::BitAccess<field_type>;
  using access1 = bitAccess_type::KeyMaker<0, 4>::value_type;
  using access2 = bitAccess_type::DependedKeyMaker<access1, 4>::value_type;
  using access3 = bitAccess_type::KeyMaker<0, 8>::value_type;

  using access4 = bitAccess_type::KeyMaker<0, 16>::value_type; // allow
  using access5 =
      bitAccess_type::DependedKeyMaker<access1, 12>::value_type; // allow

  // using access6 = bitAccess_type::KeyMaker<0, 17>::value_type; // compile
  // error! using access7 =
  //     bitAccess_type::DependedKeyMaker<access1,
  //                                      13>::value_type;       // compile
  //                                      error!
  // using access8 = bitAccess_type::KeyMaker<0, 0>::value_type; // compile
  // error! using access9 =
  //     bitAccess_type::DependedKeyMaker<access1,
  //                                      0>::value_type; // compile error!

  bitAccess_type Test;

  field_type PrintVal = Test(access1());
  std::println(std::cout, "{:b}", PrintVal);
  Test(access1()) = 0b1001;
  PrintVal = Test(access1());
  std::println(std::cout, "{:b}", PrintVal);

  PrintVal = Test(access2());
  std::println(std::cout, "{:b}", PrintVal);
  Test(access2()) = 0b11111001;
  PrintVal = Test(access2());
  std::println(std::cout, "{:b}", PrintVal);

  PrintVal = Test(access3());
  std::println(std::cout, "{:b}", PrintVal);
  Test(access3()) = 0b1110111011;
  PrintVal = Test(access3());
  std::println(std::cout, "{:b}", PrintVal);

  std::println(std::cout, "{:b}", *Test);
}
