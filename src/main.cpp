#define TAG "Main"

#include <iostream>
#include <fstream>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <cstdlib>
#include <cstdio>
#include <sys/wait.h>
#include <filesystem>
#include <cstring>
#include <vector>
#include "syscall_cpp_wrapper.h"
#include "utils.h"
#include "debug_logger.h"
#include "command_exec.h"
#include <limits.h>


struct CommandLineOptions {
    bool interactive_mode = true;
    bool debug = false;
    std::vector<std::string> command_args;
    std::string sub_command;
    std::string debug_log_file;
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
    auto pw = getpwuid(getuid());
    if (pw == nullptr) {
        DebugLogger::error("Didn't find user for uid ", getuid());
        return;
    }
    std::string username = pw->pw_name;
    std::string red_color_code = "\033[31m";
    std::string yellow_color_code = "\033[33m";
    std::string white_color_code = "\033[0m";
    while (true) {
        std::string command_buff;
        // Have to use this to print the prompt because using DebugLogger::get().print() may not print this prompt if shell
        // is not in debug mode
        char current_directory[PATH_MAX];
        memset(current_directory, 0, PATH_MAX);
        if (getcwd(current_directory, PATH_MAX) == nullptr) {
            DebugLogger::error("Failed to get current directory");
            exit(EIO);
        }
        std::cout << red_color_code << username << " " << yellow_color_code << host_name << white_color_code << ":" << std::string(current_directory) << " < ";
        std::getline(std::cin, command_buff);
        // If the user hits Ctrl+D, exit the shell
        if (std::cin.eof()) {
            DebugLogger::print("Received EOF, exiting shell");
            exit(0);
        }
        DebugLogger::print("Receive user input \"", command_buff, "\"");
        std::string executable = command_buff.substr(0, command_buff.find(" "));
        bool properly_quoted;
        parse_commands_string_and_execute(command_buff, properly_quoted);
    }
}


void run_sub_command(const CommandLineOptions &options) {
    auto executable = options.sub_command.substr(0, options.sub_command.find(" "));
    auto absolute_executable_path = resolve_complete_execute_path(executable);
    if (absolute_executable_path.length() == 0) {
        std::cerr << "No such executable file found \"" << executable << "\"" << std::endl;
        exit(ENOENT);
    }
    DebugLogger::print("exec: ", absolute_executable_path, " (from \"", executable, "\")");
    auto [properly_quoted, command_args] = split_command_into_parts(options.sub_command);
    if (!properly_quoted) {
        std::cerr << "Command is not properly quoted" << std::endl;
        exit(EINVAL);
    }
    int status;
    execv_cpp_wrapper(absolute_executable_path, command_args, &status);
}


void parse_command_line_arguments(int argc, char *argv[], CommandLineOptions &options) {
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
        } else if (std::string(argv[i]) == "-d") {
            options.debug = true;
            i++;
            continue;
        } else if (std::string(argv[i]) == "-l") {
            options.debug_log_file = std::string(argv[i + 1]);
            i += 2;
            continue;
        } else {
            std::cerr << "Unknown option: \"" << argv[i] << "\"" << std::endl;
            exit(EINVAL);
        }
    }
}

int main(int argc, char *argv[]) {
    CommandLineOptions options;
    parse_command_line_arguments(argc, argv, options);
    DebugLogger::configure(options.debug);
    if (options.debug_log_file.length() > 0) {
        DebugLogger::set_output_file(options.debug_log_file);
    }
    if (options.interactive_mode) {
        run_interactive_mode();
        // It should not reach here
    }
    run_sub_command(options);
    return 0;
}
