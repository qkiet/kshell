#define TAG "CommandExec"

#include "command_exec.h"
#include "utils.h"
#include <iostream>
#include "debug_logger.h"
#include "syscall_cpp_wrapper.h"
#include <cerrno>
#include <pwd.h>
#include <unistd.h>



int execute_command(const std::string &command, bool &properly_quoted) {
    if (strip(command).length() == 0) {
        DebugLogger::error("Command is empty");
        properly_quoted = true;
        return EINVAL;
    }
    DebugLogger::print("Executing command \"", command, "\"");
    auto [properly_quoted_result, command_parts] = split_command_into_parts(command);
    if (!properly_quoted_result) {
        DebugLogger::error("Command is not properly quoted");
        properly_quoted = false;
        return EINVAL;
    }
    properly_quoted = properly_quoted_result;
    auto executable = command_parts[0];
    if (executable == "exit") {
        exit(0);
    }
    if (executable == "cd") {
        if ((command_parts.size() < 2) || (command_parts[1] == "~")) {
            auto *pw = getpwuid(getuid());
            if (!pw) {
                DebugLogger::error("Didn't find user for uid ", getuid());
                return EINVAL;
            }
            auto home_directory = pw->pw_dir;
            if (chdir(home_directory) == -1) {
                DebugLogger::error("Failed to change directory to ", home_directory);
                return EINVAL;
            }
            return 0;
        }
        if (chdir(command_parts[1].c_str()) != 0) {
            std::cerr << "ksh: cd: " << command_parts[1] << ": No such file or directory" << std::endl;
            return EINVAL;
        }
        return 0;

    }
    auto absolute_executable_path = resolve_complete_execute_path(executable);
    DebugLogger::print("Resolved complete execute path for \"", executable, "\" to \"", absolute_executable_path, "\"");
    if (absolute_executable_path.length() == 0) {
        DebugLogger::error("No such executable file found \"", executable, "\"");
        return ENOENT;
    }
    DebugLogger::print("exec: ", absolute_executable_path, " (from \"", executable, "\")");
    command_parts.erase(command_parts.begin());
    int status;
    execv_cpp_wrapper(absolute_executable_path, command_parts, &status);
    DebugLogger::print("Command \"", command, "\" exited with status ", status);
    return status;
}
