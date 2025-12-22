#pragma once

#include <chrono>
#include <iostream>
#include <print>
#include <type_traits>

namespace maboroutu {
class timer {
private:
  using clock_type = typename std::chrono::system_clock;
  using time_point_t = typename clock_type::time_point;
  using common_type_t =
      std::common_type_t<time_point_t::duration, time_point_t::duration>;

  time_point_t _start_time_point;
  std::string _name;
  bool _do_rap_printing_when_destructed = false;

public:
  timer() : _start_time_point(std::chrono::system_clock::now()) {}
  timer(bool do_rap_printing_when_destructed)
      : _start_time_point(std::chrono::system_clock::now()),
        _do_rap_printing_when_destructed(do_rap_printing_when_destructed) {}
  timer(const timer &) = delete;
  timer(timer &&) = delete;
  auto operator=(const timer &) -> timer & = delete;
  auto operator=(timer &&) -> timer & = delete;
  timer(std::string_view const name)
      : _start_time_point(std::chrono::system_clock::now()), _name(name) {}

  ~timer() {
    // 時間計測に影響を低減するために[[likely]]を指定。
    if (_do_rap_printing_when_destructed) [[likely]] {
      println_rap();
    }
  }

  [[nodiscard]] auto rap() const noexcept -> common_type_t {
    return clock_type::now() - _start_time_point;
  }

  void println_rap() const noexcept {
    auto const stoped_time_diff = rap();

    std::println(std::cout, "TimerName: {}", _name);

    // std::printf("time: %24lld       ms\n", Time);
    std::println(
        std::cout, "time: {: 24}       ms\n",
        std::chrono::duration_cast<std::chrono::milliseconds>(stoped_time_diff)
            .count());

    // std::printf("time:    %24lld    μs\n", Time);
    std::println(
        std::cout, "time:    {: 24}    μs\n",
        std::chrono::duration_cast<std::chrono::microseconds>(stoped_time_diff)
            .count());

    // std::printf("time:       %24lld ns\n", Time);
    std::println(
        std::cout, "time:       {: 24} ns\n",
        std::chrono::duration_cast<std::chrono::nanoseconds>(stoped_time_diff)
            .count());
  }
};
} // namespace maboroutu
