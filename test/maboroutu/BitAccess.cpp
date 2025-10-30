#include <stdexcept>
#define BOOST_TEST_MAIN

#include "maboroutu/bit_access.hpp"
#include <boost/test/included/unit_test.hpp>
#include <cstdint>
#include <iostream>
#include <print>

BOOST_AUTO_TEST_CASE(bit_access) {
  using field_type = uint16_t;

  // using bit_access_type = maboroutu::bit_access<field_type>;
  // using access1 = typename bit_access_type::main_key_t<0, 4>;
  // using access2 = typename bit_access_type::key_t<access1, 4>;
  // using access3 = typename bit_access_type::main_key_t<0, 8>;
  //
  // using access4 = bit_access_type::main_key_t<0, 16>;  // allow
  // using access5 = bit_access_type::key_t<access1, 12>; // allow

  field_type value = 0;

  using accesser0 = maboroutu::bitfield_access<field_type, 1, 4>;
  using accesser1 = maboroutu::depending_bitfield_access<accesser0, 4>;
  using accesser2 = maboroutu::bitfield_access<field_type, 0, 8>;

  accesser0 field0(std::ref(value));
  accesser1 field1(std::ref(value));
  accesser2 field2(std::ref(value));

  // using access6 = bitAccess_type::KeyMaker<0, 17>::value_type; // compile
  // error! using access7 =
  //     bitAccess_type::DependedKeyMaker<access1,
  //                                      13>::value_type;       // compile
  //                                      error!
  // using access8 = bitAccess_type::KeyMaker<0, 0>::value_type; // compile
  // error! using access9 =
  //     bitAccess_type::DependedKeyMaker<access1,
  //                                      0>::value_type; // compile error!

  std::println(std::cout, "print debug!");

  field_type print_val = field0;
  BOOST_CHECK_EQUAL(print_val, 0);
  std::println(std::cout, "0:{:b}", print_val);
  field0 = 0b1001;
  print_val = field0;
  std::println(std::cout, "1:{:b}", print_val);

  print_val = field1;
  BOOST_CHECK_EQUAL(print_val, 0);
  std::println(std::cout, "2:{:b}", print_val);
  BOOST_CHECK_THROW(field1.set(0b11111001), std::overflow_error);
  BOOST_CHECK_NO_THROW(field1 = 0b11111001);
  print_val = field1;
  std::println(std::cout, "3:{:b}", print_val);
  BOOST_CHECK_NO_THROW(field1.set(0b1001));
  BOOST_CHECK_EQUAL(print_val, field1.get());
  print_val = field1;
  std::println(std::cout, "3:{:b}", print_val);

  print_val = field2;
  std::println(std::cout, "4:{:b}", print_val);
  BOOST_CHECK_THROW(field2.set(0b1110111011), std::overflow_error);
  BOOST_CHECK_NO_THROW(field2 = 0b1110111011);
  print_val = field2;
  std::println(std::cout, "5:{:b}", print_val);
  BOOST_CHECK_NO_THROW(field2.set(0b10111011));
  BOOST_CHECK_EQUAL(print_val, field2.get());
  print_val = field2;
  std::println(std::cout, "5:{:b}", print_val);

  std::println(std::cout, "6:{:b}", value);
}
