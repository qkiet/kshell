#define TAG "CommandExec"

#include "command_exec.h"
#include "utils.h"
#include <iostream>
#include "debug_logger.h"
#include "syscall_cpp_wrapper.h"
#include <cerrno>



int execute_command(const std::string &command) {
    if (strip(command).length() == 0) {
        return EINVAL;
    }
    DebugLogger::print("Executing command \"", command, "\"");
    auto executable = command.substr(0, command.find(" "));
    auto absolute_executable_path = resolve_complete_execute_path(executable);
    if (absolute_executable_path.length() == 0) {
        std::cerr << "No such executable file found \"" << executable << "\"" << std::endl;
        return ENOENT;
    }
    DebugLogger::print("exec: ", absolute_executable_path, " (from \"", executable, "\")");
    auto command_args = split_string(command);
    command_args.erase(command_args.begin());
    int status;
    execv_cpp_wrapper(absolute_executable_path, command_args, &status);
    DebugLogger::print("Command \"", command, "\" exited with status ", status);
    return status;
}
