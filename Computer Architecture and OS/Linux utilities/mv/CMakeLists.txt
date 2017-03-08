cmake_minimum_required(VERSION 3.5.1)
project(mv)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(SOURCE_FILES main.cpp)
add_executable(mv ${SOURCE_FILES})
