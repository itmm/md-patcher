cmake_minimum_required(VERSION 3.16.3)
project(mdp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(solid-require)
add_subdirectory(line-reader)
add_subdirectory(lazy-write)

add_custom_command(
	COMMAND $ENV{HOME}/bin/mdp README.md
	DEPENDS README.md
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT ${PROJECT_SOURCE_DIR}/md-patcher.cpp
)

add_executable(mdp ${PROJECT_SOURCE_DIR}/md-patcher.cpp)
target_link_libraries(mdp solid-require line-reader lazy-write)
include(CTest)
add_test(NAME run_unit-tests COMMAND ./mdp --run-only-tests)
add_test(NAME check_null_behaviour COMMAND ./mdp tests/null-test.md)

