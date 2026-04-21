#include <gtest/gtest.h>
#include "syscall_cpp_wrapper.h"
#include <vector>

TEST(SyscallCppWrapperTest, ExecvCppWrapper) {
    int status;
    int ret;
    ret = execv_cpp_wrapper("gibberish", std::vector<std::string>(), &status);
    EXPECT_EQ(ret, ENOEXEC);
    ret = execv_cpp_wrapper("/bin/ls", std::vector<std::string>(), &status);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(status, 0);
}

TEST(SyscallCppWrapperTest, ExecutePipedCommands) {
    int status;
    int ret;
    ret = execute_piped_commands(std::vector<std::tuple<std::string, std::vector<std::string>>>(), &status);
    EXPECT_EQ(ret, EINVAL);
    ret = execute_piped_commands(std::vector<std::tuple<std::string, std::vector<std::string>>>({
        {"/usr/bin/ls", std::vector<std::string>()},
    }), &status);
    EXPECT_EQ(ret, EINVAL);
    ret = execute_piped_commands(std::vector<std::tuple<std::string, std::vector<std::string>>>({
        {"/usr/bin/ls", std::vector<std::string>()},
        {"/usr/bin/xargs", std::vector<std::string>()},
    }), &status);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(status, 0);
}