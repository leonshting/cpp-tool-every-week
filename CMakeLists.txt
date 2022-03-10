cmake_minimum_required(VERSION 3.21)
project(cpp_toolbox)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

find_package(FFTW)

find_package(GTest REQUIRED)
include(GoogleTest)

include_directories(components/include)
add_subdirectory(components)

add_subdirectory(applications)
