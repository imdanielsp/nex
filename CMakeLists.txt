cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

project(nexlang)

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
