#include <iostream>
#include <fstream>
#include <errno.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <sys/wait.h>
#include <filesystem>
#include <cstring>


std::string resolve_complete_execute_path(const std::string &input_executable_path) {
    if (std::filesystem::exists(input_executable_path)) {
        std::cout << "Already found executable path!" << std::endl;
        return input_executable_path;
    }
    std::cout << "Resolve executable path from environment variable PATH..." << std::endl;
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

void execute_command(const std::string &executable, char**argument_list, size_t argument_list_len) {
    if ((nullptr == argument_list) || (argument_list_len == 0)) {
        char *args[] = {nullptr};
        execv(executable.c_str(), args);
    }
    for (int i = 0; i < argument_list_len; i++) {
        printf("argument_list[%d]=\"%s\"\n", i, argument_list[i]);
    }
    char *modifed_argument_list[argument_list_len + 2];
    char *tmp = (char*) malloc(executable.length() + 1);
    memset(tmp, 0, executable.length() + 1);
    memcpy(tmp, executable.c_str(), executable.length());
    // First argument is always the executable itself
    modifed_argument_list[0] = tmp;
    for (int i = 0; i < argument_list_len; i++) {
        modifed_argument_list[i + 1] = argument_list[i];
    }
    modifed_argument_list[argument_list_len + 1] = nullptr;
    for (int i = 0; i < argument_list_len + 2; i++) {
        printf("modifed_argument_list[%d]=\"%s\"\n", i, modifed_argument_list[i]);
    }
    pid_t child_pid = fork();
    if (child_pid == 0) {
        execv(executable.c_str(), modifed_argument_list);
    }
    free(tmp);
}

int main(int argc, char* argv[]) {
    // Find host name first
    std::ifstream host_name_file("/etc/hostname");
    if (!host_name_file.is_open()) {
        std::cerr << "Didn't find hostname" << std::endl;
        return ENOENT;
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
        auto parsed_uid = atoi(uid_str.c_str());
        auto parsed_gid = atoi(gid_str.c_str());
        if ((parsed_uid != process_uid) || (parsed_gid != process_gid)) {
            continue;
        }
        username = parsed_username;
    }
    std::string red_color_code = "\033[31m";
    std::string yellow_color_code = "\033[33m";
    std::string white_color_code = "\033[0m";
    if (argc == 1) {
        std::cout << "Not support interactive mode yet!" << std::endl;
        return EPERM;
    }
    auto executable_path = std::string(argv[1]);
    auto absolute_executable_path = resolve_complete_execute_path(executable_path);
    if (absolute_executable_path.length() == 0) {
        std::cerr << "No such file found \"" << executable_path << "\"" << std::endl;
        return ENOENT;
    }
    if (argc == 2) { // Only executable without argument
        execute_command(absolute_executable_path, nullptr, 0);
    } else {
        execute_command(absolute_executable_path, &argv[2], argc - 2);
    }

    wait(NULL);
    return 0;
}
