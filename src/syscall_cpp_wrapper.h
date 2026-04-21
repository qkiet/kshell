#ifndef KSH_SYSCALL_CPP_WRAPPER_H
#define KSH_SYSCALL_CPP_WRAPPER_H

#include <string>
#include <vector>

/**
 * Wrapper for the execv system call.
 * @param absolute_executable_path: The absolute path to the executable.
 * @param args: The arguments to pass to the executable.
 * @param status: The status of the executable. It is set to the status of the child process.
 * @return:
 * - 0 on success
 * - ENOEXEC on failure
 * - ECHILD on failure to fork
 */
int execv_cpp_wrapper(const std::string &absolute_executable_path, const std::vector<std::string> &args, int *status = nullptr);


/**
 * Execute a vector of commands in piped mode. Required at least 2 commands to execute piped commands.
 * @param command_arg_pairs: The vector of commands and their arguments.
 * @param status: The status of the child process. It is set to the status of the child process.
 * @return:
 * - 0 on success
 * - EINVAL on invalid arguments like empty vector
 * - ECHILD on failure to wait for child process
 * - EPIPE on failure to create pipe
 */
int execute_piped_commands(const std::vector<std::tuple<std::string, std::vector<std::string>>> &command_arg_pairs, int *status);

#endif // KSH_SYSCALL_CPP_WRAPPER_H