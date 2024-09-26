cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0091 NEW)

project(PeopleParty-Backend)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Get source files
set(minigame_source_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE source_files
        ${minigame_source_dir}/*.cpp
        ${minigame_source_dir}/*.h
)

# Ignore these games for now
list(FILTER source_files EXCLUDE REGEX "rps_bracket.*")
list(FILTER source_files EXCLUDE REGEX "memory_mixer.*")
list(FILTER source_files EXCLUDE REGEX "launch_party.*")
list(FILTER source_files EXCLUDE REGEX "crazy_counting.*")

# Create main target
add_executable(kokosnoot ${source_files})
