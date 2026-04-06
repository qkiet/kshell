#include <gtest/gtest.h>
#include "command_exec.h"

TEST(CommandExecTest, ExecuteCommand) {
    int status = execute_command("ls");
    EXPECT_EQ(status, 0);
    status = execute_command("ls -l");
    EXPECT_EQ(status, 0);
    status = execute_command("lslsdsds");
    EXPECT_EQ(status, ENOENT);
    status = execute_command("");
    EXPECT_EQ(status, EINVAL);
    status = execute_command(" ");
    EXPECT_EQ(status, EINVAL);
    status = execute_command("  ");
    EXPECT_EQ(status, EINVAL);
}