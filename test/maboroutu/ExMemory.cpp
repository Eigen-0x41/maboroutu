#define BOOST_TEST_MAIN

#include "maboroutu/exmemory.hpp"
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(ExMemory) {
  BOOST_TEST((maboroutu::IsPow2y<int, 0>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 1>::value) == true);
  BOOST_TEST((maboroutu::IsPow2y<int, 2>::value) == true);
  BOOST_TEST((maboroutu::IsPow2y<int, 3>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 4>::value) == true);
  BOOST_TEST((maboroutu::IsPow2y<int, 5>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 6>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 7>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 8>::value) == true);
  BOOST_TEST((maboroutu::IsPow2y<int, 9>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 10>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 11>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 12>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 13>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 14>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 15>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 16>::value) == true);
  BOOST_TEST((maboroutu::IsPow2y<int, 17>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 18>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 19>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 20>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 21>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 22>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 23>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 24>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 25>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 26>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 27>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 28>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 29>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 30>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 31>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 32>::value) == true);
  BOOST_TEST((maboroutu::IsPow2y<int, 33>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 34>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 35>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 36>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 37>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 38>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 39>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 40>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 41>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 42>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 43>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 44>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 45>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 46>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 47>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 48>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 49>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 50>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 51>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 52>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 53>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 54>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 55>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 56>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 57>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 58>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 59>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 60>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 61>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 62>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 63>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 64>::value) == true);
  BOOST_TEST((maboroutu::IsPow2y<int, 65>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 66>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 67>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 68>::value) == false);
  BOOST_TEST((maboroutu::IsPow2y<int, 69>::value) == false);
}
