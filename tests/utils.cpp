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

// ========== Nasty edge cases for split_command_into_parts ==========

// Multiple consecutive delimiters — does it collapse them or produce empties?
TEST(SplitNasty, MultipleConsecutiveSpaces) {
    auto [ok, parts] = split_command_into_parts("hello    world");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"hello", "world"}));
}

// Input is nothing but spaces
TEST(SplitNasty, OnlySpaces) {
    auto [ok, parts] = split_command_into_parts("     ");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>());
}

// Leading and trailing spaces
TEST(SplitNasty, LeadingTrailingSpaces) {
    auto [ok, parts] = split_command_into_parts("  hello  world  ");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"hello", "world"}));
}

// Quoted string preserves internal spaces
TEST(SplitNasty, QuotedStringPreservesSpaces) {
    auto [ok, parts] = split_command_into_parts("echo \"hello   world\"");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"echo", "hello   world"}));
}

// Quoted string preserves leading/trailing spaces inside quotes
TEST(SplitNasty, QuotedStringPreservesEdgeSpaces) {
    auto [ok, parts] = split_command_into_parts("echo \"  padded  \"");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"echo", "  padded  "}));
}

// Adjacent quoted and unquoted text glued together: hello"world" → helloworld
TEST(SplitNasty, AdjacentQuotedUnquoted) {
    auto [ok, parts] = split_command_into_parts("hello\"world\"");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"helloworld"}));
}

// Unquoted then quoted then unquoted glued: a"b c"d → "a" + "b c" + "d" = "ab cd"
TEST(SplitNasty, SandwichedQuote) {
    auto [ok, parts] = split_command_into_parts("a\"b c\"d");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"ab cd"}));
}

// Two adjacent quoted sections: "hello""world" → helloworld
TEST(SplitNasty, TwoAdjacentQuotedSections) {
    auto [ok, parts] = split_command_into_parts("\"hello\"\"world\"");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"helloworld"}));
}

// Multiple empty quoted strings: "" "" → two empty strings
TEST(SplitNasty, MultipleEmptyQuotedStrings) {
    auto [ok, parts] = split_command_into_parts("\"\" \"\"");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"", ""}));
}

// Empty quoted string as an argument among others
TEST(SplitNasty, EmptyQuotedAmongOthers) {
    auto [ok, parts] = split_command_into_parts("a \"\" b");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"a", "", "b"}));
}

// Single quote character — not a double quote, should be literal
TEST(SplitNasty, SingleQuoteIsLiteral) {
    auto [ok, parts] = split_command_into_parts("it's alive");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"it's", "alive"}));
}

// Tab characters — are they treated as part of the token or as whitespace?
TEST(SplitNasty, TabsInInput) {
    auto [ok, parts] = split_command_into_parts("hello\tworld");
    EXPECT_TRUE(ok);
    // Tabs are NOT the delimiter (space is), so they should be part of the token
    EXPECT_EQ(parts, std::vector<std::string>({"hello\tworld"}));
}

// Tab as custom delimiter
TEST(SplitNasty, TabAsDelimiter) {
    auto [ok, parts] = split_command_into_parts("hello\tworld", '\t');
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"hello", "world"}));
}

// Custom delimiter (comma)
TEST(SplitNasty, CommaDelimiter) {
    auto [ok, parts] = split_command_into_parts("a,b,c", ',');
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"a", "b", "c"}));
}

// Custom delimiter with quotes preserving the delimiter inside
TEST(SplitNasty, CommaDelimiterWithQuotes) {
    auto [ok, parts] = split_command_into_parts("a,\"b,c\",d", ',');
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"a", "b,c", "d"}));
}

// Pathological: delimiter char is the double-quote itself
TEST(SplitNasty, DelimiterIsQuoteChar) {
    auto [ok, parts] = split_command_into_parts("hello\"world", '"');
    // This is ambiguous/evil — just make sure it doesn't crash or hang
    (void)ok;
    (void)parts;
}

// Single character input
TEST(SplitNasty, SingleChar) {
    auto [ok, parts] = split_command_into_parts("a");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"a"}));
}

// Single space
TEST(SplitNasty, SingleSpace) {
    auto [ok, parts] = split_command_into_parts(" ");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>());
}

// Just a lone double-quote
TEST(SplitNasty, LoneQuote) {
    auto [ok, parts] = split_command_into_parts("\"");
    EXPECT_FALSE(ok);
}

// Unclosed quote at end after valid tokens
TEST(SplitNasty, UnclosedQuoteAfterTokens) {
    auto [ok, parts] = split_command_into_parts("a b \"c");
    EXPECT_FALSE(ok);
}

// Unclosed quote at beginning
TEST(SplitNasty, UnclosedQuoteAtBeginning) {
    auto [ok, parts] = split_command_into_parts("\"a b c");
    EXPECT_FALSE(ok);
}

// Quote opens and closes with nothing after, but spaces before
TEST(SplitNasty, QuotedEmptyAtEnd) {
    auto [ok, parts] = split_command_into_parts("a \"\"");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"a", ""}));
}

// Newline inside input — is it treated as a regular char?
TEST(SplitNasty, NewlineInInput) {
    auto [ok, parts] = split_command_into_parts("hello\nworld");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"hello\nworld"}));
}

// Newline inside quoted string
TEST(SplitNasty, NewlineInsideQuotes) {
    auto [ok, parts] = split_command_into_parts("\"hello\nworld\"");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"hello\nworld"}));
}

// Null byte in the middle of the string
TEST(SplitNasty, EmbeddedNullByte) {
    std::string evil("hello\0world", 11);
    auto [ok, parts] = split_command_into_parts(evil);
    // Should not crash; behavior is implementation-defined but must not UB
    (void)ok;
    (void)parts;
}

// Backslash before quote — does it escape or is it literal?
TEST(SplitNasty, BackslashBeforeQuote) {
    auto [ok, parts] = split_command_into_parts("echo \\\"hello\\\"");
    // If backslash-escaping is supported: ["echo", "\"hello\""]
    // If not: the quotes are unmatched or literal
    (void)ok;
    (void)parts;
}

// Very long input — stress test for buffer handling
TEST(SplitNasty, VeryLongInput) {
    std::string huge(100000, 'a');
    huge += " ";
    huge += std::string(100000, 'b');
    auto [ok, parts] = split_command_into_parts(huge);
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts.size(), 2);
    EXPECT_EQ(parts[0].size(), 100000);
    EXPECT_EQ(parts[1].size(), 100000);
}

// Very long quoted string
TEST(SplitNasty, VeryLongQuotedString) {
    std::string huge = "\"" + std::string(100000, 'x') + "\"";
    auto [ok, parts] = split_command_into_parts(huge);
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts.size(), 1);
    EXPECT_EQ(parts[0].size(), 100000);
}

// Lots of small tokens
TEST(SplitNasty, ManySmallTokens) {
    std::string input;
    for (int i = 0; i < 10000; i++) {
        if (i > 0) input += " ";
        input += "x";
    }
    auto [ok, parts] = split_command_into_parts(input);
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts.size(), 10000);
}

// Quote in the middle of a word: he"ll"o → hello
TEST(SplitNasty, QuoteInMiddleOfWord) {
    auto [ok, parts] = split_command_into_parts("he\"ll\"o");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"hello"}));
}

// Triple quotes: """ — one empty quoted string + one dangling quote
TEST(SplitNasty, TripleQuotes) {
    auto [ok, parts] = split_command_into_parts("\"\"\"");
    EXPECT_FALSE(ok);
}

// Four quotes: """" — two adjacent empty quoted strings
TEST(SplitNasty, FourQuotes) {
    auto [ok, parts] = split_command_into_parts("\"\"\"\"");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({""}));
}

// Space inside quotes followed by unquoted text: "a b"c → "a bc"
TEST(SplitNasty, QuotedSpaceThenUnquoted) {
    auto [ok, parts] = split_command_into_parts("\"a b\"c");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"a bc"}));
}

// Multiple quoted segments in one token: "a" "b" as separate, vs "a""b" as one
TEST(SplitNasty, QuotedSegmentsSeparatedBySpace) {
    auto [ok, parts] = split_command_into_parts("\"a\" \"b\"");
    EXPECT_TRUE(ok);
    EXPECT_EQ(parts, std::vector<std::string>({"a", "b"}));
}

// All quotes: every character is a double quote (even count)
TEST(SplitNasty, AllQuotesEven) {
    auto [ok, parts] = split_command_into_parts("\"\"\"\"\"\"");
    EXPECT_TRUE(ok);
    // Three pairs of quotes → three empty strings glued → one empty string
    EXPECT_EQ(parts, std::vector<std::string>({""}));
}

// All quotes: odd count
TEST(SplitNasty, AllQuotesOdd) {
    auto [ok, parts] = split_command_into_parts("\"\"\"\"\"");
    EXPECT_FALSE(ok);
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
