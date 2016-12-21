cmake_minimum_required(VERSION 2.8.3)
project(treeDAG)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wextra -Wall -Wno-unused-local-typedefs")

ENABLE_TESTING()
include(CTest)

add_subdirectory(treeDAG)
add_subdirectory(example)
add_subdirectory(test)
