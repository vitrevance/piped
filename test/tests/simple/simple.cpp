#include <piped.hpp>
#include <algorithm>
#include <ranges>
#include <vector>
#include <gtest/gtest.h>

using piped::$;

TEST(simple, Simple) {
  auto [result] = $[5] || (!$ * 2) || (!$ + 3);
  EXPECT_EQ(result, 13);
}

TEST(simple, Ranges) {
  std::vector<int> value = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  auto [result] = $[value] || std::ranges::views::drop(!$, 5)
                  || std::ranges::views::transform(!$,
                                                   [](auto&& x) {
                                                     return x * x;
                                                   })
                  || std::ranges::views::filter(!$, [](auto&& x) {
                       return x < 30;
                     });
  EXPECT_TRUE(std::ranges::equal(result, std::vector({25})));
}

TEST(simple, Nested) {
  int value = 10;

  auto lambda = [](int value) {
    auto [result] = $[value] || std::min(!$, 5) || std::max(!$, 2);
    return std::to_string(result);
  };

  auto [result] = $[value] || lambda(!$) || std::size(!$) || lambda(!$);
  EXPECT_EQ(result, "2");
}