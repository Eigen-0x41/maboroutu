#define BOOST_TEST_MAIN

#include "maboroutu/uniqueFile.hpp"
#include "maboroutu/streamConcepts.hpp"
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(StreamSeekConcept) {
  static_assert(maboroutu::StreamSeekConcept<maboroutu::UniqueFile>, "");
  static_assert(maboroutu::StreamInputConcept<maboroutu::UniqueFile>, "");
  static_assert(maboroutu::StreamOutputConcept<maboroutu::UniqueFile>, "");
  static_assert(maboroutu::StreamTextInputConcept<maboroutu::UniqueFile>, "");
  static_assert(maboroutu::StreamTextOutputConcept<maboroutu::UniqueFile>, "");

  BOOST_TEST(maboroutu::StreamSeekConcept<maboroutu::UniqueFile>);
  BOOST_TEST(maboroutu::StreamInputConcept<maboroutu::UniqueFile>);
  BOOST_TEST(maboroutu::StreamOutputConcept<maboroutu::UniqueFile>);
  BOOST_TEST(maboroutu::StreamTextInputConcept<maboroutu::UniqueFile>);
  BOOST_TEST(maboroutu::StreamTextOutputConcept<maboroutu::UniqueFile>);
}
