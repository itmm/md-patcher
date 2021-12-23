cmake_minimum_required(VERSION 3.18.4)
project(md-patcher)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(line-reader)
add_subdirectory(lazy-write)

add_custom_command(
	COMMAND $ENV{HOME}/bin/c-cat README.md | $ENV{HOME}/bin/md-patcher
	DEPENDS README.md
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT ${PROJECT_SOURCE_DIR}/md-patcher.cpp
)
	

add_executable(md-patcher md-patcher.cpp)
target_link_libraries(md-patcher line-reader lazy-write)

include(CTest)
add_test(NAME all COMMAND ./md-patcher --run-only-tests)

