cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0091 NEW)

project(PeopleParty-Backend)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

### Dependencies ###
find_package(uwebsockets REQUIRED)
find_package(doctest)
find_package(flatbuffers CONFIG REQUIRED)

### Main project ###
# Get source files
set(backend_source_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE source_files
        ${backend_source_dir}/*.cpp
        ${backend_source_dir}/*.h
)

# Create main target
add_executable(PeopleParty-Backend ${source_files})

# Link dependencies
target_link_libraries(PeopleParty-Backend uwebsockets::uwebsockets)
target_link_libraries(PeopleParty-Backend flatbuffers::flatbuffers)


### Tests ###
# Enable testing
include(CTest)

# Get test files
set(tests_source_dir ${CMAKE_CURRENT_SOURCE_DIR}/tests)

file(GLOB_RECURSE test_files
        ${tests_source_dir}/*.cpp
        ${tests_source_dir}/*.h
)

# Create test target
add_executable(PeopleParty-Backend-Tests ${test_files})

# Link dependencies
target_link_libraries(PeopleParty-Backend-Tests doctest::doctest)

# Enable CTest
#include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
#doctest_discover_tests(PeopleParty-Backend-Tests)
