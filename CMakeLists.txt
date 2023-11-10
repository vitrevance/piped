cmake_minimum_required(VERSION 3.25)

project(
    piped
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES OFF)

enable_testing()

add_subdirectory(piped)

option(PIPED_BUILD_TESTS "build tests" ON)

if (${PIPED_BUILD_TESTS})
    add_subdirectory(test)
endif()
