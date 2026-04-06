#define TAG "SyscallCppWrapper"

#include "syscall_cpp_wrapper.h"
#include "utils.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "debug_logger.h"

int execv_cpp_wrapper(const std::string &absolute_executable_path, const std::vector<std::string> &args, int *status) {
    DebugLogger::print("Executing executable: \"", absolute_executable_path, "\"");
    DebugLogger::print("Arguments: ");
    for (int i = 0; i < args.size(); i++) {
        DebugLogger::print("\"", args[i], "\" ");
    }
    DebugLogger::print("");

    size_t args_to_syscall_len;
    char *args_to_syscall[args.size() + 2]; // +2 for the executable path and the null terminator
    args_to_syscall[0] = (char *) absolute_executable_path.c_str();
    for (int i = 0; i < args.size(); i++) {
        args_to_syscall[i + 1] = (char *) args[i].c_str();
    }
    args_to_syscall[args.size() + 1] = nullptr;
    for (int i = 0; i < args.size() + 1; i++) {
        DebugLogger::print("args_to_syscall[", i, "] = \"", args_to_syscall[i], "\"");
    }
    pid_t child_pid = fork();
    if (child_pid == -1) {
        DebugLogger::error("Failed to fork");
        return ECHILD;
    }
    if (child_pid == 0) {
        execv(absolute_executable_path.c_str(), args_to_syscall);
        DebugLogger::error("Failed to execute executable: \"", absolute_executable_path, "\"");
        _exit(ENOEXEC);
    }

    int wait_status;
    if (waitpid(child_pid, &wait_status, 0) == -1) {
        DebugLogger::error("Failed to wait for child process");
        return ECHILD;
    }
    if (WIFEXITED(wait_status) && WEXITSTATUS(wait_status) == ENOEXEC) {
        DebugLogger::error("Executable: \"", absolute_executable_path, "\" exited with status ", WEXITSTATUS(wait_status));
        return ENOEXEC;
    }
    if (status != nullptr) {
        *status = wait_status;
    }
    return 0;
}
