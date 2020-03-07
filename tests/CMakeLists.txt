cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")

include_directories(${CMAKE_SOURCE_DIR}/lib/catch2)

include_directories(${CMAKE_SOURCE_DIR}/src)

file(
    GLOB
    SOURCE_FILES
    *.cpp *.h
)

add_executable(nexc_test ${SOURCE_FILES})
