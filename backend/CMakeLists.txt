cmake_minimum_required(VERSION 3.12)

project(PeopleParty-Backend)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

### Dependencies ###

# Get doctest
include(FetchContent)

FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG ae7a13539fb71f270b87eb2e874fbac80bc8dda2 # v2.4.6
)
FetchContent_MakeAvailable(doctest)

# uSocket and uWebSockets stuff
FetchContent_Declare(
        uSockets_content
        GIT_REPOSITORY https://github.com/uNetworking/uSockets
        GIT_TAG 833497e8e0988f7fd8d33cd4f6f36056c68d225d
        GIT_SHALLOW ON
        GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(uSockets_content)
file(GLOB_RECURSE SOURCES ${usockets_content_SOURCE_DIR}/src/*.c)
add_library(uSockets ${SOURCES})
target_include_directories(uSockets PUBLIC ${usockets_content_SOURCE_DIR}/src)
target_compile_definitions(uSockets PRIVATE LIBUS_NO_SSL UWS_NO_ZLIB)

FetchContent_Declare(
        uWebSockets_content
        GIT_REPOSITORY https://github.com/uNetworking/uWebSockets
        GIT_TAG v20.62.0
        GIT_SHALLOW ON
        GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(uWebSockets_content)
add_library(uWebSockets INTERFACE)
target_include_directories(uWebSockets INTERFACE ${uwebsockets_content_SOURCE_DIR}/src/)
target_link_libraries(uWebSockets INTERFACE uSockets)

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
target_link_libraries(PeopleParty-Backend uWebSockets)

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
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(PeopleParty-Backend-Tests)
