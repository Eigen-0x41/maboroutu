#include "maboroutu/unique_timer.hpp"
#include <array>
#include <bit>
#include <utility>
#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include "maboroutu/maboroutudef.hpp"
#include <cstddef>
#include <iostream>
#include <print>
#include <random>

static std::random_device::result_type master_seed;
static std::array<std::random_device::result_type, 128> rand_table;
static size_t rand_table_index;

static void gen_rand_table() {
  std::random_device random_device{};
  master_seed = random_device();

#pragma unroll 1
  for (auto &entry : rand_table) {
    entry = random_device();
  }
  rand_table_index = 0;
}

static auto get_probability() -> double {

  decltype(rand_table_index) const current_index = rand_table_index;
  rand_table_index = ++rand_table_index % rand_table.size();
  auto &random_access_table =
      rand_table[rand_table[rand_table_index] & 0b1111111];
  random_access_table = std::byteswap(random_access_table);
  rand_table[current_index] +=
      rand_table[rand_table_index] % ((random_access_table % 65537) + 73483);
  return static_cast<double>(rand_table[current_index]) /
         (std::random_device::max)();
}
static auto is_probability_in(double rate) noexcept -> bool {
  return get_probability() <= rate;
}

static auto target_func(double rate) noexcept -> maboroutu::ret_type<void> {
  if (is_probability_in(rate)) {
    return maboroutu::make_exception_error<std::runtime_error>(
        "exception occerd.");
  }
  return {};
}

static auto target_src_func(double rate) noexcept -> maboroutu::ret_type<void> {
  if (is_probability_in(rate)) {
    return maboroutu::make_exception_error<std::runtime_error>(
        std::source_location::current(), "exception occerd.");
  }
  return {};
}

static auto speedtest(size_t trial_count, double rate) {
  std::optional<maboroutu::timer::rap_type> rap;
  size_t successed = 0;
  size_t exceptioned = 0;

  {
    maboroutu::timer timer;
#pragma unroll 1
    for (auto count = 0; count < trial_count; count++) {
      auto result = target_func(rate);
      if (result.has_value()) {
        successed++;
      } else {
        exceptioned++;
      }
    }
    rap.emplace(timer.rap());
  }

  std::println("normal exception\n."
               "trial      : {}\n"
               "successed  : {}\n"
               "exeptioned : {}\n"
               "rate       : {}\n"
               "real rate  : {}\n"
               "all time...\n{}\n",
               trial_count, successed, exceptioned, rate,
               static_cast<double>(exceptioned) /
                   static_cast<double>(successed + exceptioned),
               *rap);
}

static auto speedsrctest(size_t trial_count, double rate) {
  std::optional<maboroutu::timer::rap_type> rap;
  size_t successed = 0;
  size_t exceptioned = 0;

  {
    maboroutu::timer timer;
#pragma unroll 1
    for (auto count = 0; count < trial_count; count++) {
      auto result = target_src_func(rate);
      if (result.has_value()) {
        successed++;
      } else {
        exceptioned++;
      }
    }
    rap.emplace(timer.rap());
  }

  std::println("add src exception\n."
               "trial      : {}\n"
               "successed  : {}\n"
               "exeptioned : {}\n"
               "rate       : {}\n"
               "real rate  : {}\n"
               "all time...\n{}\n",
               trial_count, successed, exceptioned, rate,
               static_cast<double>(exceptioned) /
                   static_cast<double>(successed + exceptioned),
               *rap);
}

BOOST_AUTO_TEST_CASE(error) {
#if !defined(NDEBUG)
  static constexpr const size_t trial_count = 1'000'000;
#else
  static constexpr const size_t trial_count = 100'000'000;
#endif /*!defined(NDEBUG) */
  gen_rand_table();

  std::println("error code:\n{}\n\n", target_func(1.0).error()->what());
  std::println("error code:\n{}\n\n", target_src_func(1.0).error()->what());

  speedtest(trial_count, 0.00);
  speedtest(trial_count, 0.01);
  speedtest(trial_count, 0.99);
  speedtest(trial_count, 1.00);

  speedsrctest(trial_count, 0.00);
  speedsrctest(trial_count, 0.01);
  speedsrctest(trial_count, 0.99);
  speedsrctest(trial_count, 1.00);
}
