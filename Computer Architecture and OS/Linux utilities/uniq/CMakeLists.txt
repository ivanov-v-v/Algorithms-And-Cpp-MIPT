cmake_minimum_required(VERSION 3.5.1)
project(uniq)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(SOURCE_FILES main.c)
add_executable(uniq ${SOURCE_FILES})
