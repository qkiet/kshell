#ifndef KSH_COMMAND_EXEC_H
#define KSH_COMMAND_EXEC_H

#include <string>
#include <vector>

/**
 * Parse a commands string (can consist of multiple commands separated by control operator like '&&' or '||' or '|') and
 * spawns multiple child processes to execute each command.
 * @param commands_string: The commands string to parse.
 * @param properly_quoted: Whether the command is properly quoted. If the command is not properly quoted, the command will not be executed and the return value will be EINVAL.
 * @return:
 * - 0 on success
 * - EINVAL if the command is not properly quoted
 * - ENOENT if the executable file is not found
 * - ENOEXEC if the executable file is not executable
 */
int parse_commands_string_and_execute(const std::string &commands_string, bool &properly_quoted);

#endif // KSH_COMMAND_EXEC_H
