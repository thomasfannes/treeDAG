include_directories(${PROJECT_SOURCE_DIR})

find_package(Boost 1.53 COMPONENTS timer REQUIRED)

add_executable(separatorCache separatorCache.cpp)
target_link_libraries(separatorCache treeDAG)

add_executable(decomposer decomposer.cpp)
target_link_libraries(decomposer treeDAG ${Boost_LIBRARIES})
