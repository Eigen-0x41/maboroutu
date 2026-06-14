#pragma once

#include <chrono>
#include <concepts>
#include <format>
#include <iostream>
#include <print>
#include <string_view>
#include <type_traits>

namespace maboroutu {

class timer_rap_point_base {};

template <class T>
concept timer_rap_point_printable = std::derived_from<T, timer_rap_point_base>;

} // namespace maboroutu

template <maboroutu::timer_rap_point_printable RapTypeT>
struct std::formatter<RapTypeT> : std::formatter<const char *> {
  static auto format(RapTypeT const &value, std::format_context &ctx) {
    return std::format_to(ctx.out(),
                          "Timer Name: {}\n\n"
                          "time: {: 24}       ms\n"
                          "time:    {: 24}    μs\n"
                          "time:       {: 24} ns\n",
                          value.name(), value.milliseconds().count(),
                          value.microseconds().count(),
                          value.nanoseconds().count());
  }
} __attribute__((packed));

namespace maboroutu {

template <class TimePointT>
// [[timer_rap_point]]
class timer_rap_point : public timer_rap_point_base {
public: /*STRUCT_FIELD*/
  using time_point_t = TimePointT;

protected:
private:
  std::string _name;
  std::common_type_t<typename time_point_t::duration,
                     typename time_point_t::duration>
      _value;

  /*--:  *IMPLIMENT_FIELD*/
protected:
public:
  timer_rap_point() = delete;
  timer_rap_point(timer_rap_point const &) = default;
  timer_rap_point(timer_rap_point &&) = default;
  timer_rap_point(
      std::string name,
      std::common_type_t<typename time_point_t::duration,
                         typename time_point_t::duration> const &value)
      : _name(std::move(name)), _value(value) {}
  ~timer_rap_point() = default;

  auto operator=(timer_rap_point const &rhs) -> timer_rap_point & = default;
  auto operator=(timer_rap_point &&rhs) -> timer_rap_point & = default;

  [[nodiscard]] auto name() const noexcept { return _name; }
  [[nodiscard]] auto milliseconds() const noexcept {
    return std::chrono::duration_cast<std::chrono::milliseconds>(_value);
  }
  [[nodiscard]] auto microseconds() const noexcept {
    return std::chrono::duration_cast<std::chrono::microseconds>(_value);
  }
  [[nodiscard]] auto nanoseconds() const noexcept {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(_value);
  }
};

class timer {
public:
  using clock_type = typename std::chrono::system_clock;
  using time_point_t = typename clock_type::time_point;
  using common_type_t =
      std::common_type_t<time_point_t::duration, time_point_t::duration>;
  using rap_type = timer_rap_point<time_point_t>;

private:
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
    // 時間計測への影響を低減するために[[likely]]を指定。
    if (_do_rap_printing_when_destructed) [[likely]] {
      std::println("{}", rap());
    }
  }

  [[nodiscard]] auto rap() const noexcept -> rap_type {
    return {_name, clock_type::now() - _start_time_point};
  }
};

} // namespace maboroutu
