cmake_minimum_required(VERSION 3.6)
project(TimSort)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O2")

set(SOURCE_FILES main.cpp headers/timsort_stack.h headers/timsort_merge.h headers/timsort_params.h headers/timsort.h)
add_executable(TimSort ${SOURCE_FILES} headers/timsort_stack.h headers/timsort_merge.h headers/timsort_params.h headers/timsort.h)
