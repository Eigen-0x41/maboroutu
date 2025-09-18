#define BOOST_TEST_MAIN
#include "maboroutu/linkNode.hpp"
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <print>
#include <ranges>

BOOST_AUTO_TEST_CASE(LinkNode) {
  using continer_type = maboroutu::IndexedLinkNodeContiner<int>;
  continer_type::continer_type Continer;
  continer_type V0(Continer);

  V0.Sentinel.insert(0);
  V0.Sentinel.insert(1);
  V0.Sentinel.insert(2);
  V0.Sentinel.insert(3);

  continer_type::node_type *Node;
  Node = &V0.Sentinel;

  BOOST_TEST_MESSAGE("Single Link Node Test.");
  for (auto const I : std::views::iota(0, 4)) {
    Node = &Node->next();
    BOOST_CHECK_EQUAL(**Node, I);
    std::println(std::cout, "{}", **Node);
  }
  for (int I = 3; I > -1; I--) {
    BOOST_CHECK_EQUAL(**Node, I);
    std::println(std::cout, "{}", **Node);
    Node = &Node->previous();
  }
  for (auto const I : std::views::iota(0, 4)) {
    BOOST_CHECK_EQUAL(*Continer[I], I);
    std::println(std::cout, "{}", *Continer[I]);
  }

  BOOST_TEST_MESSAGE("Mix Link Node Test.");
  Continer.clear();
  continer_type V1(Continer);

  V0.Sentinel.insert(0);
  V1.Sentinel.insert(1);
  V0.Sentinel.insert(2);
  V1.Sentinel.insert(3);
  V0.Sentinel.insert(4);
  V1.Sentinel.insert(5);
  V0.Sentinel.insert(6);
  V1.Sentinel.insert(7);

  Node = &V0.Sentinel;
  for (auto const I : std::views::iota(0, 4)) {
    Node = &Node->next();
    BOOST_CHECK_EQUAL(**Node, (I * 2));
    std::println(std::cout, "{}", **Node);
  }
  Node = &V1.Sentinel;
  for (auto const I : std::views::iota(0, 4)) {
    Node = &Node->next();
    BOOST_CHECK_EQUAL(**Node, 1 + (I * 2));
    std::println(std::cout, "{}", **Node);
  }
  for (auto const I : std::views::iota(0, 8)) {
    BOOST_CHECK_EQUAL(*Continer[I], I);
    std::println(std::cout, "{}", *Continer[I]);
  }

  BOOST_TEST_MESSAGE("Iterator test.");
  {
    int Count = 0;
    for (auto const I : V0.Sentinel) {
      BOOST_CHECK_EQUAL(I, Count);
      std::println(std::cout, "{}", I);
      Count += 2;
    }

    Count = 1;
    for (auto const I : V1.Sentinel) {
      BOOST_CHECK_EQUAL(I, Count);
      std::println(std::cout, "{}", I);
      Count += 2;
    }
  }

  BOOST_TEST_MESSAGE("Move node test.");
  assert(V1.Sentinel.next().move(2));
  Node = &V0.Sentinel;
  for (auto I = 0; I < 3; I++) {
    Node = &Node->next();
    BOOST_CHECK_EQUAL(**Node, I);
    std::println(std::cout, "{}", **Node);
  }
  auto Ite = V0.Sentinel.end();
  for (auto I = 0; I < 3; I++) {
    Ite++;
    BOOST_CHECK_EQUAL(*Ite, I);
    std::println(std::cout, "{}", *Ite);
  }

  BOOST_TEST_MESSAGE("Iterator to Node test.");
  Ite = ++V1.Sentinel.begin();
  assert(continer_type::node_type::toNode(Ite).move(6));
  Ite = V0.Sentinel.end();
  for (auto I = 6; I > 3; I--) {
    Ite--;
    BOOST_CHECK_EQUAL(*Ite, I);
    std::println(std::cout, "{}", *Ite);
  }

  BOOST_TEST_MESSAGE("tryAt test.");
  auto FalseAt = V0.tryAt(8);
  assert(!FalseAt);
  std::println(std::cout, "{}", FalseAt.error().what());

  auto TrueAt = V0.tryAt(7);
  if (TrueAt) [[likely]] {
    **TrueAt = 100;
    std::println(std::cout, "{}", **TrueAt);
    std::println(std::cout, "{}", *V1.Sentinel.previous());
  }
}
