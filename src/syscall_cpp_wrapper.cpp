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


int execute_piped_commands(const std::vector<std::tuple<std::string, std::vector<std::string>>> &command_arg_pairs, int *status) {
    if (command_arg_pairs.size() < 2) {
        DebugLogger::error("At least 2 commands are required to execute piped commands");
        return EINVAL;
    }
    std::vector<pid_t> child_pids;
    int current_pipe_fd[2];
    int previous_pipe_fd[2];
    pid_t last_command_pid;

    for (auto it = command_arg_pairs.begin(); it != command_arg_pairs.end(); it++) {
        auto [command, args] = *it;
        DebugLogger::print("Executing executable: \"", command, "\"");
        if (args.size() > 0) {
            DebugLogger::print("Arguments: ");
            for (int i = 0; i < args.size(); i++) {
                DebugLogger::print("\"", args[i], "\" ");
            }
            DebugLogger::print("");
        }
        size_t args_to_syscall_len;
        args_to_syscall_len = args.size() + 2; // +2 for the executable path and the null terminator
        char *args_to_syscall[args_to_syscall_len];
        args_to_syscall[0] = (char *) command.c_str();
        for (size_t i = 0; i < args.size(); i++) {
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
        // Onlly create new pipe if not the last command
        if (it + 1 != command_arg_pairs.end()) {
            if (pipe(current_pipe_fd) == -1) {
                DebugLogger::error("Failed to create first pipe");
                return EPIPE;
            }
        }
        child_pids.push_back(child_pid);
        if (child_pid == 0) {
            if (it == command_arg_pairs.begin()) {
                DebugLogger::print("Redirecting stdout of current command to the write end of the pipe");
                // Redirect stdout of current command to the write end of the pipe
                close(current_pipe_fd[0]);
                dup2(current_pipe_fd[1], STDOUT_FILENO);
                close(current_pipe_fd[1]);
            } else if (it + 1 != command_arg_pairs.end()) {
                // Redirect stdout of current command to the write end of the pipe
                // and also redirect the read end of the previous pipe to the stdin of the current command
                DebugLogger::print("Redirecting the read end of the previous pipe to the stdin of the current command and redirecting stdout of current command to the write end of the pipe");
                close(previous_pipe_fd[1]);
                dup2(previous_pipe_fd[0], STDIN_FILENO);
                close(previous_pipe_fd[0]);
                close(current_pipe_fd[0]);
                dup2(current_pipe_fd[1], STDOUT_FILENO);
                close(current_pipe_fd[1]);
            } else {
                // For last command, only redirect the read end of the previous pipe to the stdin of the current command
                DebugLogger::print("Redirecting the read end of the previous pipe to the stdin of the current command");
                close(previous_pipe_fd[1]);
                dup2(previous_pipe_fd[0], STDIN_FILENO);
                close(previous_pipe_fd[0]);
            }
            execv(command.c_str(), args_to_syscall);
            DebugLogger::error("Failed to execute executable: \"", command, "\"");
            return ENOEXEC;
        }
        DebugLogger::print("Child pid: ", child_pid);
        // Now time to cache the pipe fd for the next command and also close previous pipe if exist
        // For first command, only cache the pipe fd
        if (it == command_arg_pairs.begin()) {
            previous_pipe_fd[0] = current_pipe_fd[0];
            previous_pipe_fd[1] = current_pipe_fd[1];
            DebugLogger::print("Cached the pipe fd for the next command [0]: ", previous_pipe_fd[0], " [1]: ", previous_pipe_fd[1]);
        // For middle commands, close previous pipe and cache the current pipe fd
        } else if (it + 1 != command_arg_pairs.end()) {
            close(previous_pipe_fd[0]);
            close(previous_pipe_fd[1]);
            DebugLogger::print("Closed previous pipe [0]: ", previous_pipe_fd[0], " [1]: ", previous_pipe_fd[1]);
            previous_pipe_fd[0] = current_pipe_fd[0];
            previous_pipe_fd[1] = current_pipe_fd[1];
            DebugLogger::print("Cached the pipe fd for the next command [0]: ", previous_pipe_fd[0], " [1]: ", previous_pipe_fd[1]);
        } else {
            // For last command, only close previous pipe
            close(previous_pipe_fd[0]);
            close(previous_pipe_fd[1]);
            DebugLogger::print("Closed previous pipe [0]: ", previous_pipe_fd[0], " [1]: ", previous_pipe_fd[1]);
        }
        // Store last command pid
        if (it + 1 == command_arg_pairs.end()) {
            last_command_pid = child_pid;
            DebugLogger::print("Last command pid: ", last_command_pid);
        }
    }
    // Debug: let's wait for all commands to finish
    for (auto it = child_pids.begin(); it != child_pids.end(); it++) {
        int debug_wait_status;
        if (waitpid(*it, &debug_wait_status, 0) == -1) {
            DebugLogger::error("Failed to wait for command");
            return ECHILD;
        }
        DebugLogger::print("Command pid: ", *it, " exited with status ", WEXITSTATUS(debug_wait_status));
        if (*it == last_command_pid) {
            if (WIFEXITED(debug_wait_status) && WEXITSTATUS(debug_wait_status) == ENOEXEC) {
                DebugLogger::error("Last command exited with status ", WEXITSTATUS(debug_wait_status));
                return ENOEXEC;
            }
            if (status != nullptr) {
                *status = debug_wait_status;
            }
            DebugLogger::print("Last command exited with status ", WEXITSTATUS(debug_wait_status));
        }
    }
    return 0;
}