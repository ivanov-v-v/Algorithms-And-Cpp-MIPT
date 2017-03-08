cmake_minimum_required(VERSION 3.3)
 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -Werror")

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
 
# Link runTests with what we want to test and the GTest and pthread library
set(SOURCE_FILES main.cpp src/deque.cpp headers/timsort_stack.h headers/timsort_merge.h headers/timsort_params.h headers/timsort.h headers/rational.h)
add_executable(runTests ${SOURCE_FILES} main.cpp src/deque.cpp headers/timsort_stack.h headers/timsort_merge.h headers/timsort_params.h headers/timsort.h headers/rational.h)
target_link_libraries(runTests ${GTEST_LIBRARIES} pthread)
