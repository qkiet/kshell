#include <cerrno>
#include <string>
#include <gtest/gtest.h>
#include "command_exec.h"

// ── Existing basic tests ─────────────────────────────────────────────

TEST(CommandExecTest, ExecuteCommand) {
    bool properly_quoted;
    int status = execute_command("ls", properly_quoted);
    EXPECT_TRUE(properly_quoted);
    EXPECT_EQ(status, 0);
    status = execute_command("ls -l", properly_quoted);
    EXPECT_EQ(status, 0);
    status = execute_command("lslsdsds", properly_quoted);
    EXPECT_TRUE(properly_quoted);
    EXPECT_EQ(status, ENOENT);
    status = execute_command("", properly_quoted);
    EXPECT_EQ(status, EINVAL);
    status = execute_command(" ", properly_quoted);
    EXPECT_EQ(status, EINVAL);
    status = execute_command("  ", properly_quoted);
    EXPECT_TRUE(properly_quoted);
    EXPECT_EQ(status, EINVAL);
    status = execute_command("\"ls\"", properly_quoted);
    EXPECT_TRUE(properly_quoted);
    EXPECT_EQ(status, 0);
    status = execute_command("\"ls", properly_quoted);
    EXPECT_FALSE(properly_quoted);
    EXPECT_EQ(status, EINVAL);
    status = execute_command("\"ls\" \"-l\"", properly_quoted);
    EXPECT_TRUE(properly_quoted);
    EXPECT_EQ(status, 0);
    status = execute_command("ls\"\"", properly_quoted);
    EXPECT_TRUE(properly_quoted);
    EXPECT_EQ(status, 0);
}

// ── Quoting nightmares ──────────────────────────────────────────────

TEST(CommandExecTest, QuoteOnlyAtEnd) {
    bool pq;
    int s = execute_command("ls \"", pq);
    EXPECT_FALSE(pq);
    EXPECT_EQ(s, EINVAL);
}

TEST(CommandExecTest, EmptyQuotedString) {
    bool pq;
    int s = execute_command("\"\"", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, ENOENT);
}

TEST(CommandExecTest, QuotedSpacesAsArg) {
    bool pq;
    int s = execute_command("echo \"   \"", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, 0);
}

TEST(CommandExecTest, QuotedSpacesAsCommand) {
    bool pq;
    int s = execute_command("\"   \"", pq);
    EXPECT_TRUE(pq);
    EXPECT_TRUE(s == EINVAL || s == ENOENT);
}

TEST(CommandExecTest, NestedQuoteLookalike) {
    // echo "hello "world" bye" — inner quote closes the first pair
    // tokens: [echo] [hello ] [world] [ bye] — 4 quotes, all paired
    bool pq;
    int s = execute_command("echo \"hello \"world\" bye\"", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, 0);
}

TEST(CommandExecTest, ConsecutiveQuotePairs) {
    bool pq;
    int s = execute_command("\"\"\"\"\"\"", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, ENOENT);
}

TEST(CommandExecTest, OddNumberOfQuotes) {
    bool pq;
    int s = execute_command("\"\"\"", pq);
    EXPECT_FALSE(pq);
    EXPECT_EQ(s, EINVAL);
}

TEST(CommandExecTest, EmptyQuoteBeforeCommand) {
    bool pq;
    int s = execute_command("\"\"ls", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, 0);
}

TEST(CommandExecTest, QuotedCommandWithTrailingSpaceInside) {
    bool pq;
    int s = execute_command("\"ls \"", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, ENOENT);
}

TEST(CommandExecTest, QuotedCommandWithLeadingSpaceInside) {
    bool pq;
    int s = execute_command("\" ls\"", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, ENOENT);
}

TEST(CommandExecTest, BackslashBeforeQuote) {
    bool pq;
    int s = execute_command("echo \\\"hello\\\"", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, 0);
}

// ── Whitespace weirdness ────────────────────────────────────────────

TEST(CommandExecTest, LeadingAndTrailingSpaces) {
    bool pq;
    int s = execute_command("   ls   ", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, 0);
}

TEST(CommandExecTest, ManySpacesBetweenArgs) {
    bool pq;
    int s = execute_command("echo       hello       world", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, 0);
}

// ── Command resolution ──────────────────────────────────────────────

TEST(CommandExecTest, CommandReturnsNonZero) {
    bool pq;
    int s = execute_command("false", pq);
    EXPECT_TRUE(pq);
    EXPECT_NE(s, 0);
    EXPECT_NE(s, ENOENT);
    EXPECT_NE(s, EINVAL);
}

TEST(CommandExecTest, ChildExitsZero) {
    bool pq;
    int s = execute_command("true", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, 0);
}

TEST(CommandExecTest, AbsolutePathCommand) {
    bool pq;
    int s = execute_command("/bin/ls", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, 0);
}

TEST(CommandExecTest, AbsolutePathNotFound) {
    bool pq;
    int s = execute_command("/no/such/binary", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, ENOENT);
}

TEST(CommandExecTest, RelativePathNotFound) {
    bool pq;
    int s = execute_command("./nonexistent_binary", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, ENOENT);
}

// ── Adversarial inputs ──────────────────────────────────────────────

TEST(CommandExecTest, NullByteInCommand) {
    bool pq;
    std::string cmd("ls\0 -l", 6);
    int s = execute_command(cmd, pq);
    EXPECT_TRUE(s == 0 || s == ENOENT || s == EINVAL);
}

TEST(CommandExecTest, CommandIsJustEmptyQuotePairs) {
    bool pq;
    int s = execute_command("\"\" \"\" \"\"", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, ENOENT);
}

TEST(CommandExecTest, VeryLongCommand) {
    bool pq;
    std::string cmd = "echo " + std::string(200000, 'A');
    int s = execute_command(cmd, pq);
    EXPECT_TRUE(pq);
    (void)s;
}

TEST(CommandExecTest, ExceedArgMax) {
    bool pq;
    std::string cmd = "echo " + std::string(3 * 1024 * 1024, 'B');
    int s = execute_command(cmd, pq);
    EXPECT_TRUE(pq);
    (void)s;
}

// ── Quote + whitespace combos ───────────────────────────────────────

TEST(CommandExecTest, QuotedEmptyBetweenRealArgs) {
    bool pq;
    int s = execute_command("echo \"\" hello", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, 0);
}

TEST(CommandExecTest, SpaceAroundQuotedCommand) {
    bool pq;
    int s = execute_command("  \"ls\"  ", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, 0);
}

TEST(CommandExecTest, AdjacentQuotedStringsGlued) {
    bool pq;
    int s = execute_command("echo \"hello\"\"world\"", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, 0);
}

TEST(CommandExecTest, UnmatchedQuoteAfterCommand) {
    bool pq;
    int s = execute_command("echo hello\"", pq);
    EXPECT_FALSE(pq);
    EXPECT_EQ(s, EINVAL);
}

TEST(CommandExecTest, UnmatchedQuoteBeforeCommand) {
    bool pq;
    int s = execute_command("\"echo hello", pq);
    EXPECT_FALSE(pq);
    EXPECT_EQ(s, EINVAL);
}

TEST(CommandExecTest, QuoteInMiddleOfWord) {
    // ec"ho hel"lo → should become token [echo hello] as the command name
    bool pq;
    int s = execute_command("ec\"ho hel\"lo", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, ENOENT);
}

TEST(CommandExecTest, OnlyQuotesAndSpaces) {
    bool pq;
    int s = execute_command("  \"\"  \"\"  ", pq);
    EXPECT_TRUE(pq);
    EXPECT_EQ(s, ENOENT);
}