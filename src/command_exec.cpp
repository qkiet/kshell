#define TAG "CommandExec"

#include "command_exec.h"
#include "utils.h"
#include <iostream>
#include "debug_logger.h"
#include "syscall_cpp_wrapper.h"
#include <cerrno>



int execute_command(const std::string &command, bool &properly_quoted) {
    if (strip(command).length() == 0) {
        DebugLogger::error("Command is empty");
        properly_quoted = true;
        return EINVAL;
    }
    DebugLogger::print("Executing command \"", command, "\"");
    auto [properly_quoted_result, command_args] = split_command_into_parts(command);
    if (!properly_quoted_result) {
        DebugLogger::error("Command is not properly quoted");
        properly_quoted = false;
        return EINVAL;
    }
    properly_quoted = properly_quoted_result;
    auto executable = command_args[0];
    auto absolute_executable_path = resolve_complete_execute_path(executable);
    DebugLogger::print("Resolved complete execute path for \"", executable, "\" to \"", absolute_executable_path, "\"");
    if (absolute_executable_path.length() == 0) {
        DebugLogger::error("No such executable file found \"", executable, "\"");
        return ENOENT;
    }
    DebugLogger::print("exec: ", absolute_executable_path, " (from \"", executable, "\")");
    command_args.erase(command_args.begin());
    int status;
    execv_cpp_wrapper(absolute_executable_path, command_args, &status);
    DebugLogger::print("Command \"", command, "\" exited with status ", status);
    return status;
}
