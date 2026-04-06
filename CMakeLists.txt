cmake_minimum_required(VERSION 3.22)
project(ksh)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/syscall_cpp_wrapper.cpp
    src/utils.cpp
    src/command_exec.cpp
)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    ksh_test
    tests/utils.cpp
    tests/syscall_cpp_wrapper.cpp
    tests/command_exec.cpp
    src/utils.cpp
    src/syscall_cpp_wrapper.cpp
    src/command_exec.cpp
)

target_include_directories(ksh_test PRIVATE src)

target_link_libraries(
  ksh_test
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ksh_test)