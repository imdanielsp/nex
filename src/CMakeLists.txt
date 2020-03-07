cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")

file(
    GLOB
    SOURCE_FILES
    *.cpp
    *.hpp
)

add_executable(nexc ${SOURCE_FILES})
