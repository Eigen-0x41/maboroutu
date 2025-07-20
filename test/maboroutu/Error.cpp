#include <cstddef>
#define BOOST_TEST_MAIN

#include "maboroutu/maboroutuDef.hpp"
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <print>

maboroutu::ret<int> TestFunc(bool DoError) {
  if (DoError) {
    return maboroutu::makeRetErr<maboroutu::ret<>::error_type>(
        maboroutu::ret<void>::error_type::categoly_type::Logic,
        "Test Exception");
  }
  return 10;
}
maboroutu::ret<ptrdiff_t> convFunc(bool DoError) {
  if (DoError) {
    return maboroutu::convRetErr<ptrdiff_t>(TestFunc(true));
  }
  return TestFunc(false);
}

BOOST_AUTO_TEST_CASE(Error) {
  {
    auto Val = TestFunc(false);
    BOOST_CHECK_EQUAL(Val.has_value(), true);
    std::println(std::cout, "Value: {}", *Val);

    Val = TestFunc(true);
    BOOST_CHECK_EQUAL(Val.has_value(), false);
    std::println(std::cout, "ErrMess: {}", Val.error().what());
  }

  {
    auto Val = TestFunc(false);
    BOOST_CHECK_EQUAL(Val.has_value(), true);
    std::println(std::cout, "Value: {}", *Val);

    Val = convFunc(true);
    BOOST_CHECK_EQUAL(Val.has_value(), false);
    std::println(std::cout, "ErrMess: {}", Val.error().what());
  }
}
