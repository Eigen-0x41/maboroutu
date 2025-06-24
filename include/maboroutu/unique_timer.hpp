#pragma once

#include <chrono>
#include <iostream>
#include <print>

class UniqueTimer {
private:
  using clock_type = typename std::chrono::system_clock;

  clock_type::time_point StartTimer;
  std::string Name;

public:
  UniqueTimer() : StartTimer(std::chrono::system_clock::now()), Name("") {}
  UniqueTimer(std::string_view const Name)
      : StartTimer(std::chrono::system_clock::now()), Name(Name) {}

  virtual ~UniqueTimer() {
    clock_type::time_point const EndTime = clock_type::now();

    long long Time = std::chrono::duration_cast<std::chrono::milliseconds>(
                         EndTime - StartTimer)
                         .count();
    std::println(std::cout, "TimerName: {}", Name);

    // std::printf("time: %24lld       ms\n", Time);
    std::println(std::cout, "time: {: 24}       ms\n", Time);

    Time = std::chrono::duration_cast<std::chrono::microseconds>(EndTime -
                                                                 StartTimer)
               .count();
    // std::printf("time:    %24lld    μs\n", Time);
    std::println(std::cout, "time:    {: 24}    μs\n", Time);

    Time = std::chrono::duration_cast<std::chrono::nanoseconds>(EndTime -
                                                                StartTimer)
               .count();
    // std::printf("time:       %24lld ns\n", Time);
    std::println(std::cout, "time:       {: 24} ns\n", Time);
  }
};
