cmake_minimum_required(VERSION 3.10)
project(tog)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 20)

set(CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "${CXX_FLAGS}")

# TODO there's probably a better way to do this with CMake
include_directories("libs/")
add_executable(${PROJECT_NAME} src/tog.cpp)
