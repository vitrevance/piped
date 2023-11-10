#include <piped.hpp>
#include <gtest/gtest.h>
#include <string>

using piped::$;

template <typename T>
auto DeductionFunc(T callback) {
  return callback(1);
}

TEST(templates, Deduction) {
  auto lambda = [](int arg) {
    return arg + 1;
  };

  auto result = $[lambda] || DeductionFunc(!$) || lambda(!$) || $;
  EXPECT_EQ(result, 3);
}

template <typename T>
auto DecltypeFunc(std::decay_t<T> value) {
  return std::string(value) + " world";
}

TEST(templates, Decltype) {
  auto result = $["hello"] || DecltypeFunc<decltype(!$)>(!$) || $;
  EXPECT_EQ(result, "hello world");
}
