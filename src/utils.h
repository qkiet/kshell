#ifndef KSH_UTILS_H
#define KSH_UTILS_H

#include <string>
#include <vector>

void vector_to_null_term_char_pointer_list(const std::vector<std::string> &vec, char ***ptr_list);
void free_char_pointer_list(char **ptr_list[], size_t len);
void debug_vector(const std::vector<std::string> &vec);
std::string resolve_complete_execute_path(const std::string &input_executable_path);
std::string strip(const std::string &src, char delim = ' ');
std::vector<std::string> split_string(const std::string &str, char delim = ' ');


#endif // KSH_UTILS_H