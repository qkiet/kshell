#include <iostream>
#include <fstream>
#include <errno.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <sys/wait.h>
#include <filesystem>
#include <cstring>
#include <vector>

void split_string(const std::string &str, std::vector<std::string> &vec, char delim = ' ');

struct CommandLineOptions {
    bool interactive_mode = true;
    std::vector<std::string> command_args;
    std::string sub_command;
};


void debug_vector(const std::vector<std::string> &vec) {
    for (int i = 0; i < vec.size(); i++) {
        std::cout << "Vector " << i << " is " << vec[i] << std::endl;
    }
}


void char_point_list_to_vector(const char ***ptr_list, size_t len, std::vector<std::string> &vec) {
    if (len == 0) {
        return;
    }
    for (int i = 0; i < len; i++) {
        vec.push_back(std::string((*ptr_list)[i]));
    }
}


void vector_to_null_term_char_pointer_list(const std::vector<std::string> &vec, char ***ptr_list) {
    *ptr_list = (char**) malloc(vec.size() + 1);
    for (int i = 0; i < vec.size(); i++) {
        (*ptr_list)[i] = (char *) malloc(vec[i].length() + 1);
        memset((*ptr_list)[i], 0, vec[i].length() + 1);
        memcpy((*ptr_list)[i], vec[i].c_str(), vec[i].length());
    }
    (*ptr_list)[vec.size()] = nullptr;
}


void free_char_pointer_list(char **ptr_list[], size_t len) {
    for (int i = 0; i < len; i++) {
        free((*ptr_list)[i]);
    }
    free(*ptr_list);
}


void execv_cpp_wrapper(const std::string executable, const std::vector<std::string> &args) {
    std::cout << "Executing executable: \"" << executable << "\"" << std::endl;
    std::cout << "Arguments: ";
    for (int i = 0; i < args.size(); i++) {
        std::cout << "\"" << args[i] << "\" ";
    }
    std::cout << std::endl;
    char **args_to_syscall;

    if (args.size() == 0) {
        std::cerr << "args must have at least one argument - the absolute path to executable path itself" << std::endl;
        return;
    }
    vector_to_null_term_char_pointer_list(args, &args_to_syscall);
    pid_t child_pid = fork();
    if (child_pid == -1) {
        std::cerr << "Failed to fork" << std::endl;
        return;
    }
    if (child_pid == 0) {
        execv(executable.c_str(), args_to_syscall);
        std::cerr << "Failed to execute executable: \"" << executable << "\"" << std::endl;
        return;
    }
    wait(NULL);
    free_char_pointer_list(&args_to_syscall, args.size() + 1);
}


std::string resolve_complete_execute_path(const std::string &input_executable_path) {
    if (std::filesystem::exists(input_executable_path)) {
        std::cout << "Already found executable path!" << std::endl;
        return input_executable_path;
    }
    if (NULL == std::getenv("PATH")) {
        std::cout << "Not found \"PATH\" value!" << std::endl;
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


void strip(const std::string &src, std::string &dst, char delim) {
    // Let's copy dst to src first
    dst = src;
    size_t begin_of_delim = -1;
    size_t end_of_delim = -1;
    for (size_t i = 0; i < src.length();) {
        if (dst[i] != delim) {
            // This is the end of the delim group. Time to replace it with empty string
            if ((begin_of_delim != -1) && (begin_of_delim != end_of_delim)) {
                std::cout << "Replace! begin_of_delim=" << begin_of_delim << "end_of_delim=" << end_of_delim << " with " << std::string("") << std::endl;
                dst.replace(begin_of_delim, end_of_delim - begin_of_delim + 1, std::string(1, delim));
                // Update the index after the replacement
                i = begin_of_delim;
                begin_of_delim = -1;
                end_of_delim = -1;
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
            std::cout << "Update the end index of the delim group to " << i << std::endl;
            end_of_delim = i;
            i++;
            continue;
        }
        // this is the first delim of a new delim group
        std::cout << "New delim group found at index " << i << std::endl;
        end_of_delim = i;
        begin_of_delim = i;
        // If there is one delim at the end of the string, strip it too!
        if (begin_of_delim == (dst.length() - 1)) {
            std::cout << "Replace! begin_of_delim=" << begin_of_delim << " with " << std::string("") << std::endl;
            dst.replace(begin_of_delim, 1, std::string(""));
            i = begin_of_delim;
            begin_of_delim = -1;
            end_of_delim = -1;
            continue;
        }
        i++;
    }
}


void split_string(const std::string &str, std::vector<std::string> &vec, char delim) {
    if (str.find(delim) == std::string::npos) {
        vec.push_back(str);
        return;
    }
    // Sanitize delimit first if there are consecutive deliminators occasion
    std::string sanitized_str;
    strip(str, sanitized_str, delim);
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
}


void run_interactive_mode() {
     // Find host name first
    std::ifstream host_name_file("/etc/hostname");
    if (!host_name_file.is_open()) {
        std::cerr << "Didn't find hostname" << std::endl;
        return;
    }
    std::string host_name;
    host_name_file >> host_name;
    // Find what is this user name. For now, we simply inherit the uid and gid from calling process, which is parent shell
    // @todo: if let user execute this shell directly (from /etc/passwd), find the way to know what exactly user
    // this shell should serve.
    uid_t process_uid = getuid();
    gid_t process_gid = getgid();
    std::ifstream passwd_file("/etc/passwd");
    std::string line;
    // Typical line of /etc/passwd looks like this:
    // root:x:0:0:root:/root:/bin/bash
    std::string username;
    while (std::getline(passwd_file, line)) {
        std::string parsed_username = line.substr(0, line.find(":"));
        auto begin_of_uid = line.find(":") + 3; // drop the next ":x:" characters
        auto end_of_uid = line.find(":", begin_of_uid);
        auto begin_of_gid = end_of_uid + 1; // skip the current ":" character
        auto end_of_gid = line.find(":", begin_of_gid);
        auto uid_str = line.substr(begin_of_uid, end_of_uid - begin_of_uid);
        auto gid_str = line.substr(begin_of_gid, end_of_gid - begin_of_gid);
        auto parsed_uid = std::stoi(uid_str.c_str());
        auto parsed_gid = std::stoi(gid_str.c_str());
        if ((parsed_uid != process_uid) || (parsed_gid != process_gid)) {
            continue;
        }
        username = parsed_username;
    }
    std::string red_color_code = "\033[31m";
    std::string yellow_color_code = "\033[33m";
    std::string white_color_code = "\033[0m";
    while (true) {
        std::string command_buff;
        std::cout << red_color_code << username << " " << yellow_color_code << host_name << white_color_code << " < ";
        std::getline(std::cin, command_buff);
        if (std::cin.eof()) {
            exit(0);
        }
        std::cout << "Receive user input \"" << command_buff << "\"" << std::endl;
        std::string executable = command_buff.substr(0, command_buff.find(" "));
        if (executable == std::string("exit")) {
            exit(0);
        }
        std::cout << "Executable: \"" << executable << "\"" << std::endl;
        std::vector<std::string> command_args;
        std::string absolute_executable_path;
        absolute_executable_path = resolve_complete_execute_path(executable);
        if (absolute_executable_path.length() == 0) {
            std::cerr << "No such file found \"" << executable << "\"" << std::endl;
            continue;
        }
        split_string(command_buff, command_args, ' ');
        execv_cpp_wrapper(absolute_executable_path, command_args);
    }
}


void run_sub_command(const CommandLineOptions &options) {
    auto executable = options.sub_command.substr(0, options.sub_command.find(" "));
    auto absolute_executable_path = resolve_complete_execute_path(executable);
    if (absolute_executable_path.length() == 0) {
        std::cerr << "No such executable file found \"" << executable << "\"" << std::endl;
        exit(ENOENT);
    }
    std::vector<std::string> command_args;
    split_string(options.sub_command, command_args, ' ');
    execv_cpp_wrapper(absolute_executable_path, command_args);
}


void parse_command_line_arguments(int argc, char* argv[], CommandLineOptions &options) {
    for (int i = 1; i < argc;) {
        if (std::string(argv[i]) == "-c") {
            options.interactive_mode = false;
            if (i + 1 >= argc) {
                std::cerr << "No sub command found after -c" << std::endl;
                exit(EINVAL);
            }
            if (argv[i + 1][0] == '-') {
                std::cerr << "Invalid sub command: \"" << argv[i + 1] << "\"" << std::endl;
                exit(EINVAL);
            }
            options.sub_command = std::string(argv[i + 1]);
            i += 2;
            continue;
        }
    }
}

int main(int argc, char* argv[]) {
    CommandLineOptions options;
    parse_command_line_arguments(argc, argv, options);
    if (options.interactive_mode) {
        run_interactive_mode();
        // It should not reach here
    }
    run_sub_command(options);
    return 0;
}
