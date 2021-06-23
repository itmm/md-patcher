cmake_minimum_required(VERSION 3.19)
project(md-patcher)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(line-reader)
add_subdirectory(lazy-write)

add_executable(md-patcher md-patcher.cpp)
target_link_libraries(md-patcher line-reader lazy-write)