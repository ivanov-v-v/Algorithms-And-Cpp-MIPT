cmake_minimum_required(VERSION 3.6)
project(Matrix)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp headers/rational.h)
add_executable(Matrix ${SOURCE_FILES})