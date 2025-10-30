#define BOOST_TEST_MAIN

#include "maboroutu/uniqueFile.hpp"
#include "maboroutu/streamConcepts.hpp"
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(StreamSeekConcept) {
  static_assert(maboroutu::StreamSeekConcept<maboroutu::UniqueFile>, "");
  static_assert(maboroutu::stream_input_concept<maboroutu::unique_file>, "");
  static_assert(maboroutu::stream_output_concept<maboroutu::unique_file>, "");
  static_assert(maboroutu::stream_text_input_concept<maboroutu::unique_file>, "");
  static_assert(maboroutu::stream_text_output_concept<maboroutu::unique_file>, "");

  BOOST_TEST(maboroutu::StreamSeekConcept<maboroutu::UniqueFile>);
  BOOST_TEST(maboroutu::stream_input_concept<maboroutu::unique_file>);
  BOOST_TEST(maboroutu::stream_output_concept<maboroutu::unique_file>);
  BOOST_TEST(maboroutu::stream_text_input_concept<maboroutu::unique_file>);
  BOOST_TEST(maboroutu::stream_text_output_concept<maboroutu::unique_file>);
}
