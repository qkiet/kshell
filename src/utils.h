#ifndef KSH_UTILS_H
#define KSH_UTILS_H

#include <string>
#include <vector>
#include <tuple>

void debug_vector(const std::vector<std::string> &vec);
std::string resolve_complete_execute_path(const std::string &input_executable_path);
std::string strip(const std::string &src, char delim = ' ');
std::tuple<bool, std::vector<std::string>> split_command_into_parts(const std::string &cmd, char delim = ' ');
bool is_properly_quoted(const std::string &str);

#endif // KSH_UTILS_H