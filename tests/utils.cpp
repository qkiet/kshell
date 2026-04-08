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
TEST(UtilTest, SplitCommandIntoParts) {
    auto [properly_quoted, parts] = split_command_into_parts("hello world");
    EXPECT_TRUE(properly_quoted);
    EXPECT_EQ(parts.size(), 2);
    EXPECT_EQ(parts, std::vector<std::string>({"hello", "world"}));
    auto [properly_quoted2, parts2] = split_command_into_parts("");
    EXPECT_TRUE(properly_quoted2);
    EXPECT_EQ(parts2, std::vector<std::string>());
    auto [properly_quoted3, parts3] = split_command_into_parts("hello \"world\"");
    EXPECT_TRUE(properly_quoted3);
    EXPECT_EQ(parts3.size(), 2);
    EXPECT_EQ(parts3, std::vector<std::string>({"hello", "world"}));
    auto [properly_quoted4, parts4] = split_command_into_parts("hello \"world");
    EXPECT_FALSE(properly_quoted4);
    auto [properly_quoted5, parts5] = split_command_into_parts("\"\"");
    EXPECT_TRUE(properly_quoted5);
    EXPECT_EQ(parts5, std::vector<std::string>({""}));
}

TEST(UtilTest, IsProperlyQuoted) {
    EXPECT_TRUE(is_properly_quoted(""));
    EXPECT_TRUE(is_properly_quoted("\"\""));
    EXPECT_FALSE(is_properly_quoted("\""));
    EXPECT_TRUE(is_properly_quoted("hello world"));
    EXPECT_FALSE(is_properly_quoted("hello world\""));
    EXPECT_FALSE(is_properly_quoted("\"hello world"));
    EXPECT_TRUE(is_properly_quoted("\"hello world\""));
    EXPECT_FALSE(is_properly_quoted("\"\"hello world\""));
    EXPECT_FALSE(is_properly_quoted("\"hello world\"\""));
    EXPECT_TRUE(is_properly_quoted("\"\"hello world\"\""));
    EXPECT_FALSE(is_properly_quoted("\"hello\" \"world"));
    EXPECT_FALSE(is_properly_quoted("\"hello \"world\""));
    EXPECT_TRUE(is_properly_quoted("\"hello\" \"world\""));
}
