cmake_minimum_required(VERSION 3.5.1)
project(ps)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wuninitialized")

set(SOURCE_FILES main.c)
add_executable(ps ${SOURCE_FILES})
