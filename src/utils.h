#ifndef KSH_UTILS_H
#define KSH_UTILS_H

#include <string>
#include <vector>
#include <tuple>

void debug_vector(const std::vector<std::string> &vec);
std::string resolve_complete_execute_path(const std::string &input_executable_path);
/**
 * Strip string to eliminate leading and trailing spaces, as well as consecutive spaces.
 * @param src: The string to strip.
 * @return: The stripped string.
 */
std::string strip(const std::string &src);
/**
 * Split string into parts using the delimiter.
 * @param str: The string to split.
 * @param delim: The delimiter to split the string by.
 * @return: A tuple containing a boolean indicating if the string is properly quoted and a vector of split substrings.
 */
std::tuple<bool, std::vector<std::string>> split_string_into_parts(const std::string &str, const std::string &delim);
bool is_properly_quoted(const std::string &str);

#endif // KSH_UTILS_H