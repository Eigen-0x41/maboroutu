#include <boost/test/tools/old/interface.hpp>
#include <utility>
#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <maboroutu/reference_counter.hpp>
#include <print>

template <>
struct std::formatter<maboroutu::reference_counter>
    : std::formatter<const char *> {
  auto format(maboroutu::reference_counter const &value,
              std::format_context &ctx) const {
    return std::format_to(ctx.out(), "[.count(): {}, .unique(): {}]",
                          value.count(), value.unique());
  }
};
template <>
struct std::formatter<maboroutu::reference_watcher>
    : std::formatter<const char *> {
  auto format(maboroutu::reference_watcher const &value,
              std::format_context &ctx) const {
    return std::format_to(ctx.out(), "[.count(): {}, .expired(): {}]",
                          value.count(), value.expired());
  }
};

template <class Ost, class T>
static void print_test_expander(Ost &ost, T const &printing) {
  std::println(ost, "{}", printing);
}
template <class Ost, class T, class... Args>
static void print_test_expander(Ost &ost, T const &printing, Args &&...args) {
  std::println(ost, "{}", printing);
  print_test_expander(ost, std::forward<Args>(args)...);
}
template <class Ost, class... Args>
static void print_test(Ost &ost, size_t sequence, Args &&...args) {
  std::println(ost, "sequence {}.", sequence);
  print_test_expander(ost, std::forward<Args>(args)...);
  std::println(ost, "");
}

static void boost_test(maboroutu::reference_counter const &value, long count,
                       bool unique) {
  BOOST_CHECK_EQUAL(value.count(), count);
  BOOST_CHECK_EQUAL(value.unique(), unique);
}
static void boost_test(maboroutu::reference_watcher const &value, long count,
                       bool expired) {
  BOOST_CHECK_EQUAL(value.count(), count);
  BOOST_CHECK_EQUAL(value.expired(), expired);
}

void case0() {
  size_t sequence = 0;

  std::println("case0\n");

  maboroutu::reference_watcher last_ref_watcher{};
  {
    {
      maboroutu::reference_counter ref_counter0{};
      print_test(std::cout, sequence++, ref_counter0);
      boost_test(ref_counter0, 1, true);

      {
        maboroutu::reference_watcher ref_watcher0(ref_counter0);
        print_test(std::cout, sequence++, ref_counter0, ref_watcher0);
        boost_test(ref_counter0, 1, true);
        boost_test(ref_watcher0, 1, false);

        maboroutu::reference_counter ref_counter1(ref_counter0);
        print_test(std::cout, sequence++, ref_counter0, ref_counter1,
                   ref_watcher0);
        boost_test(ref_counter0, 2, false);
        boost_test(ref_counter1, 2, false);
        boost_test(ref_watcher0, 2, false);

        last_ref_watcher = ref_counter0;
        print_test(std::cout, sequence++, ref_counter0, ref_counter1,
                   ref_watcher0, last_ref_watcher);

        boost_test(ref_counter0, 2, false);
        boost_test(ref_counter1, 2, false);
        boost_test(last_ref_watcher, 2, false);
        boost_test(ref_watcher0, 2, false);
      }
      print_test(std::cout, sequence++, ref_counter0, last_ref_watcher);
      boost_test(ref_counter0, 1, true);
      boost_test(last_ref_watcher, 1, false);

      maboroutu::reference_watcher ref_watcher0(ref_counter0);
      print_test(std::cout, sequence++, ref_counter0, ref_watcher0,
                 last_ref_watcher);
      boost_test(ref_counter0, 1, true);
      boost_test(ref_watcher0, 1, false);
      boost_test(last_ref_watcher, 1, false);

      maboroutu::reference_counter ref_counter1(ref_counter0);
      print_test(std::cout, sequence++, ref_counter0, ref_counter1,
                 ref_watcher0, last_ref_watcher);
      boost_test(ref_counter0, 2, false);
      boost_test(ref_counter1, 2, false);
      boost_test(ref_watcher0, 2, false);
      boost_test(last_ref_watcher, 2, false);
    }

    print_test(std::cout, sequence++, last_ref_watcher);
    boost_test(last_ref_watcher, 0, true);

    maboroutu::reference_counter last_ref_counter{last_ref_watcher};
    print_test(std::cout, sequence++, last_ref_counter, last_ref_watcher);
    boost_test(last_ref_counter, 1, true);
    boost_test(last_ref_watcher, 1, false);
  }

  print_test(std::cout, sequence++, last_ref_watcher);
  boost_test(last_ref_watcher, 0, true);

  std::println();
}
void case1() {
  size_t sequence = 0;

  std::println("case1\n");

  maboroutu::reference_counter ref_counter0{};
  maboroutu::reference_counter ref_counter1{};
  maboroutu::reference_counter ref_counter2{ref_counter0};
  maboroutu::reference_watcher ref_watcher0{ref_counter1};
  maboroutu::reference_watcher ref_watcher1{ref_counter2};

  print_test(std::cout, sequence++, ref_counter0, ref_counter1, ref_counter2,
             ref_watcher0, ref_watcher1);
  boost_test(ref_counter0, 2, false);
  boost_test(ref_counter1, 1, true);
  boost_test(ref_counter2, 2, false);
  boost_test(ref_watcher0, 1, false);
  boost_test(ref_watcher1, 2, false);

  ref_counter1.swap(ref_counter0);
  print_test(std::cout, sequence++, ref_counter0, ref_counter1, ref_counter2,
             ref_watcher0, ref_watcher1);
  boost_test(ref_counter0, 1, true);
  boost_test(ref_counter1, 2, false);
  boost_test(ref_counter2, 2, false);
  boost_test(ref_watcher0, 1, false);
  boost_test(ref_watcher1, 2, false);

  ref_watcher0.swap(ref_watcher1);
  print_test(std::cout, sequence++, ref_counter0, ref_counter1, ref_counter2,
             ref_watcher0, ref_watcher1);
  boost_test(ref_counter0, 1, true);
  boost_test(ref_counter1, 2, false);
  boost_test(ref_counter2, 2, false);
  boost_test(ref_watcher0, 2, false);
  boost_test(ref_watcher1, 1, false);

  ref_counter1.reset();
  print_test(std::cout, sequence++, ref_counter0, ref_counter1, ref_counter2,
             ref_watcher0, ref_watcher1);
  boost_test(ref_counter0, 1, true);
  boost_test(ref_counter1, 1, true);
  boost_test(ref_counter2, 1, true);
  boost_test(ref_watcher0, 1, false);
  boost_test(ref_watcher1, 1, false);

  ref_watcher0.reset();
  print_test(std::cout, sequence++, ref_counter0, ref_counter1, ref_counter2,
             ref_watcher0, ref_watcher1);
  boost_test(ref_counter0, 1, true);
  boost_test(ref_counter1, 1, true);
  boost_test(ref_counter2, 1, true);
  boost_test(ref_watcher0, 0, true);
  boost_test(ref_watcher1, 1, false);

  std::println();
}

BOOST_AUTO_TEST_CASE(reference_counter) {
  case0();
  case1();
}
