#define TAG "Utils"

#include "utils.h"
#include <cstring>
#include <iostream>
#include <filesystem>
#include "debug_logger.h"
#include <stack>


void debug_vector(const std::vector<std::string> &vec) {
    for (int i = 0; i < vec.size(); i++) {
        DebugLogger::print("Vector ", i, " is ", vec[i]);
    }
}

std::string resolve_complete_execute_path(const std::string &input_executable_path) {
    DebugLogger::print("Resolving complete execute path for \"", input_executable_path, "\"");
    if (input_executable_path.length() == 0) {
        DebugLogger::error("Input executable path is empty");
        return std::string("");
    }
    if (std::filesystem::exists(input_executable_path)) {
        DebugLogger::print("Already found executable path!");
        return input_executable_path;
    }
    if (NULL == std::getenv("PATH")) {
        DebugLogger::error("Not found \"PATH\" value!");
        return std::string("");
    }
    auto path_value = std::string(std::getenv("PATH"));
    auto begin_path_element_value = 0;
    auto end_colon_pos = path_value.find(":", begin_path_element_value);
    while (end_colon_pos != std::string::npos) {
        auto path_element_value = path_value.substr(begin_path_element_value, end_colon_pos - begin_path_element_value);
        auto absolute_executable_path = path_element_value + "/" + input_executable_path;
        if (std::filesystem::exists(absolute_executable_path)) {
            return absolute_executable_path;
        }
        begin_path_element_value = end_colon_pos + 1;
        end_colon_pos = path_value.find(":", begin_path_element_value);
    }
    return std::string();
}

std::string strip(const std::string &src, char delim) {
    // Let's copy dst to src first
    DebugLogger::print("Stripping string \"", src, "\" with delimiter \"", delim, "\"");
    std::string dst = src;
    size_t begin_of_delim = -1;
    size_t end_of_delim = -1;
    bool delim_group_at_beginning = false;
    for (size_t i = 0; i < src.length();) {
        if (dst[i] != delim) {
            // This is the end of the delim group. Time to replace it with empty string
            if (begin_of_delim != -1 && begin_of_delim != end_of_delim) {
                DebugLogger::print("Replace! begin_of_delim=", begin_of_delim, ", end_of_delim=", end_of_delim, " with \"", std::string(1, delim), "\"");
                dst.replace(begin_of_delim, end_of_delim - begin_of_delim + 1, std::string(1, delim));
                // Update the index after the replacement
                i = begin_of_delim;
                begin_of_delim = -1;
                end_of_delim = -1;
                continue;
            }
            if (begin_of_delim != -1 && delim_group_at_beginning) {
                DebugLogger::print("Replace! begin_of_delim=", begin_of_delim, ", end_of_delim=", end_of_delim, " with \"", std::string(""), "\"");
                dst.replace(begin_of_delim, end_of_delim - begin_of_delim + 1, std::string(""));
                i = begin_of_delim;
                begin_of_delim = -1;
                end_of_delim = -1;
                delim_group_at_beginning = false;
                continue;
            }
            // Otherwise, just move to the next character
            i++;
            begin_of_delim = -1;
            end_of_delim = -1;
            continue;
        }
        if (begin_of_delim != -1) {
            // Update the end index of the delim group
            DebugLogger::print("Update the end index of the delim group to ", i);
            end_of_delim = i;
            i++;
            // Special case: if the delim is at the end of the string, strip it too!
            if (i >= dst.length()) {
                DebugLogger::print("Replace! begin_of_delim=", begin_of_delim, ", end_of_delim=", end_of_delim, " with \"\"");
                dst.replace(begin_of_delim, end_of_delim - begin_of_delim + 1, std::string(""));
            }
            continue;
        }
        // this is the first delim of a new delim group
        DebugLogger::print("New delim group found at index ", i);
        if (i == 0) {
            delim_group_at_beginning = true;
        }
        begin_of_delim = i;
        end_of_delim = i;
        // If there is one delim at the end of the string, strip it too!
        if (begin_of_delim == (dst.length() - 1)) {
            DebugLogger::print("Replace! begin_of_delim=", begin_of_delim, ", end_of_delim=", end_of_delim, " with ", std::string(""));
            dst.replace(begin_of_delim, 1, std::string(""));
            i = begin_of_delim;
            begin_of_delim = -1;
            end_of_delim = -1;
            continue;
        }
        i++;
    }
    return dst;
}


std::tuple<bool, std::vector<std::string>> split_command_into_parts(const std::string &cmd, char delim) {
    DebugLogger::print("Splitting string \"", cmd, "\" with delimiter \"", delim, "\" into parts");
    if (cmd.length() == 0) {
        return std::make_tuple(true, std::vector<std::string>());
    }
    std::vector<std::string> vec;
    // Sanitize delimit first if there are consecutive deliminators occasion
    auto sanitized_str = strip(cmd, delim);
    bool is_in_quote = false;
    std::string current_part;
    for (auto it = sanitized_str.begin(); it != sanitized_str.end(); it++) {
        char c = *it;
        if (c == '"') {
            // Quote at the beginning of the string, change quote mode to true immediately
            if (it == sanitized_str.begin()) {
                is_in_quote = true;
                continue;
            }
            auto last_char = *(it - 1);
            // Quote has escaped, that mean add the quote to the current part
            if (last_char == '\\') {
                current_part += c;
                continue;
            }
            // Quote has not escaped, toggle quote mode
            is_in_quote = !is_in_quote;
            continue;
        }
        if (c == delim) {
            // Delim is added to the current part if we are in a quote, that's what double quote are for!
            if (is_in_quote) {
                current_part += c;
                continue;
            }
            DebugLogger::print("Adding part \"", current_part, "\" to the result vector");
            vec.push_back(current_part);
            current_part.clear();
            continue;
        }
        // Otherwise, just add the character to the current part
        current_part += c;
    }
    // At the end of the loop, if we are in a quote, that means the command is not properly quoted
    // return false and an empty vector
    if (is_in_quote) {
        return std::make_tuple(false, std::vector<std::string>());
    }
    // Last part is not added to the vector, add it now
    DebugLogger::print("Adding last part \"", current_part, "\" to the result vector");
    vec.push_back(current_part);
    return std::make_tuple(true, vec);
}

bool is_properly_quoted(const std::string &str) {
    if (str.length() == 0) {
        return true;
    }
    std::stack<char> quote_stack;
    for (const auto &c : str) {
        if (c == '"') {
            if (quote_stack.empty()) {
                quote_stack.push(c);
            } else {
                quote_stack.pop();
            }
        }
    }
    return quote_stack.empty();
}
