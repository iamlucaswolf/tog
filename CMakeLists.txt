cmake_minimum_required(VERSION 3.10)

add_definitions(-std=c++17)

set(CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "${CXX_FLAGS}")

project(tog)

add_executable(tog src/tog.cpp)