#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include "maboroutu/maboroutudef.hpp"
#include <cstddef>
#include <iostream>
#include <print>

using maboroutu::ret_type;

namespace {
constexpr const char *error_message = "Test Unexpect Message.";

auto test_func(bool do_error) -> ret_type<int> {
  if (do_error) {
    return ret_type<>::unexpected_type{maboroutu::ret_type<int>::error_type(
        maboroutu::ret_type<>::error_type::categoly_type::logic,
        error_message)};
  }
  return 10;
}
auto conv_func(bool do_error) -> ret_type<ptrdiff_t> {
  if (do_error) {
    return test_func(true);
  }
  return test_func(false);
}
} // namespace

BOOST_AUTO_TEST_CASE(error) {
  {
    auto val = test_func(false);
    BOOST_ASSERT(val.has_value());
    std::println(std::cout, "Value: {}", *val);

    val = test_func(true);
    BOOST_ASSERT(!val.has_value());
    BOOST_CHECK_EQUAL(val.error().what(), error_message);
    BOOST_ASSERT(val.error().categoly() ==
                 ret_type<>::error_type::categoly_type::logic);
    BOOST_ASSERT(val.error().descript() ==
                 ret_type<>::error_type::descript_type::none);

    std::println(std::cout, "ErrMess: {}", val.error().what());
  }

  {
    auto val = test_func(false);
    BOOST_ASSERT(val.has_value());
    std::println(std::cout, "Value: {}", *val);

    val = conv_func(true);
    BOOST_ASSERT(!val.has_value());
    BOOST_CHECK_EQUAL(val.error().what(), error_message);
    BOOST_ASSERT(val.error().categoly() ==
                 ret_type<>::error_type::categoly_type::logic);
    BOOST_ASSERT(val.error().descript() ==
                 ret_type<>::error_type::descript_type::none);
    std::println(std::cout, "ErrMess: {}", val.error().what());
  }
}
