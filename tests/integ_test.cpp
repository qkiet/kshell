#include <gtest/gtest.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <limits.h>
#include <iostream>


class IntegTest : public ::testing::Test {
protected:
    pid_t shell_pid = -1;
    int input_fd = -1;
    int output_fd = -1;
    const ::testing::TestInfo* test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    void SetUp() override {
        char current_directory[PATH_MAX];
        memset(current_directory, 0, PATH_MAX);
        if (getcwd(current_directory, PATH_MAX) == nullptr) {
            FAIL() << "Failed to get current directory";
        }
        char shell_path[PATH_MAX];
        memset(shell_path, 0, PATH_MAX);
        strncpy(shell_path, current_directory, PATH_MAX);
        // @todo: use better way to get the path of shell
        strncat(shell_path, "/build/ksh", PATH_MAX);

        int stdin_pipe[2];
        int stdout_pipe[2];
        if (pipe(stdin_pipe) == -1) {
            FAIL() << "Failed to create stdin pipe";
        }
        if (pipe(stdout_pipe) == -1) {
            close(stdin_pipe[0]);
            close(stdin_pipe[1]);
            FAIL() << "Failed to create stdout pipe";
        }

        char test_log_file[PATH_MAX];
        memset(test_log_file, 0, PATH_MAX);
        snprintf(test_log_file, PATH_MAX, "ksh_integ_test_%s_%s.log", test_info->test_suite_name(), test_info->name());

        shell_pid = fork();
        if (shell_pid == -1) {
            FAIL() << "Failed to fork child process";
        }
        if (shell_pid == 0) {
            close(stdin_pipe[1]);
            dup2(stdin_pipe[0], STDIN_FILENO);
            close(stdin_pipe[0]);

            close(stdout_pipe[0]);
            dup2(stdout_pipe[1], STDOUT_FILENO);
            close(stdout_pipe[1]);

            char *args[] = {shell_path, (char *)"-d", (char *)"-l", test_log_file, nullptr};
            execv(shell_path, args);
            _exit(1);
        }

        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
        input_fd = stdin_pipe[1];
        output_fd = stdout_pipe[0];
        std::cout << "Done setting up" << std::endl;
    }

    void TearDown() override {
        if (input_fd != -1) close(input_fd);
        if (output_fd != -1) close(output_fd);
        if (shell_pid > 0) {
            kill(shell_pid, SIGTERM);
            waitpid(shell_pid, nullptr, 0);
        }
        std::cout << "Done tearing down" << std::endl;
    }
};

TEST_F(IntegTest, SimpleCommandExecution) {
    char buffer[1024];

    memset(buffer, 0, sizeof(buffer));
    read(output_fd, buffer, sizeof(buffer) - 1);
    std::cout << "Prompt: " << buffer << std::endl;

    write(input_fd, "echo hello\n", 11);

    memset(buffer, 0, sizeof(buffer));
    read(output_fd, buffer, sizeof(buffer) - 1);
    std::cout << "Output: " << buffer << std::endl;
    EXPECT_EQ(std::string(buffer), "hello\n");
}

TEST_F(IntegTest, ExitCommandExecution) {
    write(input_fd, "exit\n", 5);
    int status;
    waitpid(shell_pid, &status, 0);
    EXPECT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0);
}

TEST_F(IntegTest, SendEOFToShell) {
    close(input_fd);
    int status;
    waitpid(shell_pid, &status, 0);
    EXPECT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0);
}
