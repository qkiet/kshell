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
#include "syscall_cpp_wrapper.h"
#include "utils.h"


struct CommandLineOptions {
    bool interactive_mode = true;
    std::vector<std::string> command_args;
    std::string sub_command;
};


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
        // If the user hits Ctrl+D, exit the shell
        if (std::cin.eof()) {
            exit(0);
        }
        std::cout << "Receive user input \"" << command_buff << "\"" << std::endl;
        std::string executable = command_buff.substr(0, command_buff.find(" "));
        // Self-explanatory, "exit" to exit the shell
        if (executable == std::string("exit")) {
            exit(0);
        }
        std::cout << "Executable: \"" << executable << "\"" << std::endl;
        std::string absolute_executable_path;
        absolute_executable_path = resolve_complete_execute_path(executable);
        if (absolute_executable_path.length() == 0) {
            std::cerr << "No such file found \"" << executable << "\"" << std::endl;
            continue;
        }
        auto command_args = split_string(command_buff);
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
    auto command_args = split_string(options.sub_command);
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
