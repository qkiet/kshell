#ifndef KSH_COMMAND_EXEC_H
#define KSH_COMMAND_EXEC_H

#include <string>
#include <vector>

/**
 * Execute a command in a child process.
 * @param command: The command to execute. This command allows for double quotes to be used to enclose arguments that contains spaces.
 * @param properly_quoted: Whether the command is properly quoted. If the command is not properly quoted, the command will not be executed and the return value will be EINVAL.
 * @return:
 * - The status of the command
 * - EINVAL if the command is not properly quoted
 * - ENOENT if the executable file is not found
 */
int execute_command(const std::string &command, bool &properly_quoted);

#endif // KSH_COMMAND_EXEC_H
