#include <gtest/gtest.h>
#include "utils.h"

// Test the strip function
TEST(UtilTest, Strip) {
  std::string str = "hello  world";
  std::string dst = strip(str, ' ');
  EXPECT_EQ(dst, "hello world");
  str = " hello  world  ";
  dst = strip(str, ' ');
  EXPECT_EQ(dst, "hello world");
  str = "  ";
  dst = strip(str);
  EXPECT_EQ(dst, std::string(""));
}

// Test the split_string function
TEST(UtilTest, SplitString) {
  std::string str = "hello world";
  std::vector<std::string> vec;
  vec = split_string(str, ' ');
  EXPECT_EQ(vec.size(), 2);
  EXPECT_EQ(vec[0], "hello");
  EXPECT_EQ(vec[1], "world");
  std::string str2 = "";
  std::vector<std::string> vec2;
  vec2 = split_string(str2, ' ');
  EXPECT_EQ(vec2.size(), 0);
}
