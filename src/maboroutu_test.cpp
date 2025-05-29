#include <iostream>
#include <print>

#include "maboroutu.hpp"

int main(int argc, char *argv[]) {
  maboroutu::UnorderdIndexKeyMap<int> Test;

  auto I1 = Test.emplace(0);
  auto I2 = Test.emplace(1);
  auto I3 = Test.emplace(2);
  auto I4 = Test.emplace(3);

  std::println(std::cout, "i:{} v:{}", I1, Test.at(I1));
  std::println(std::cout, "i:{} v:{}", I2, Test.at(I2));
  std::println(std::cout, "i:{} v:{}", I3, Test.at(I3));
  std::println(std::cout, "i:{} v:{}", I4, Test.at(I4));

  Test.erase(I2);
  Test.erase(I4);
  I2 = Test.emplace(14);
  I4 = Test.emplace(15);

  std::println(std::cout, "i:{} v:{}", I1, Test.at(I1));
  std::println(std::cout, "i:{} v:{}", I2, Test.at(I2));
  std::println(std::cout, "i:{} v:{}", I3, Test.at(I3));
  std::println(std::cout, "i:{} v:{}", I4, Test.at(I4));

  Test.erase(I1);
  Test.erase(I2);
  Test.erase(I4);

  I1 = Test.emplace(26);
  I2 = Test.emplace(27);
  I4 = Test.emplace(28);

  std::println(std::cout, "i:{} v:{}", I1, Test.at(I1));
  std::println(std::cout, "i:{} v:{}", I2, Test.at(I2));
  std::println(std::cout, "i:{} v:{}", I3, Test.at(I3));
  std::println(std::cout, "i:{} v:{}", I4, Test.at(I4));

  // std::println(std::cout,"",);

  return 0;
}
