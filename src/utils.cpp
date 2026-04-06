#define TAG "Utils"

#include "utils.h"
#include <cstring>
#include <iostream>
#include <filesystem>
#include "debug_logger.h"


void debug_vector(const std::vector<std::string> &vec) {
    for (int i = 0; i < vec.size(); i++) {
        DebugLogger::print("Vector ", i, " is ", vec[i]);
    }
}

std::string resolve_complete_execute_path(const std::string &input_executable_path) {
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


std::vector<std::string> split_string(const std::string &str, char delim) {
    DebugLogger::print("Splitting string \"", str, "\" with delimiter \"", delim, "\"");
    if (str.length() == 0) {
        return std::vector<std::string>();
    }
    std::vector<std::string> vec;
    if (str.find(delim) == std::string::npos) {
        vec.push_back(str);
        return vec;
    }
    // Sanitize delimit first if there are consecutive deliminators occasion
    auto sanitized_str = strip(str, delim);
    size_t begin_substr = 0;
    size_t next_delim_pos = sanitized_str.find(delim);
    do {
        std::string sub_str = sanitized_str.substr(begin_substr, next_delim_pos - begin_substr);
        vec.push_back(sub_str);
        begin_substr = next_delim_pos + 1;
        next_delim_pos = sanitized_str.find(delim, begin_substr);
    } while(next_delim_pos != std::string::npos);
    if (begin_substr < sanitized_str.length()) {
        std::string sub_str = sanitized_str.substr(begin_substr, sanitized_str.length() - begin_substr);
        vec.push_back(sub_str);
    }
    return vec;
}
