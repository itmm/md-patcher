cmake_minimum_required(VERSION 3.16.3)
project(md-patcher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(line-reader)
add_subdirectory(lazy-write)

add_custom_command(
	COMMAND $ENV{HOME}/bin/c-cat README.md | $ENV{HOME}/bin/md-patcher
	DEPENDS README.md
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT ${PROJECT_SOURCE_DIR}/md-patcher.cpp
)
	

add_executable(md-patcher ${PROJECT_SOURCE_DIR}/md-patcher.cpp)
target_link_libraries(md-patcher line-reader lazy-write)
target_link_libraries(md-patcher stdc++fs)
include(CTest)
add_test(NAME all COMMAND ./md-patcher --run-only-tests)

