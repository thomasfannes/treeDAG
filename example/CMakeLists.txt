include_directories(${PROJECT_SOURCE_DIR})

find_package(Boost 1.53 COMPONENTS timer system REQUIRED)

add_executable(separatorCache separatorCache.cpp)
target_link_libraries(separatorCache treeDAG ${Boost_LIBRARIES})

add_executable(decomposer decomposer.cpp)
target_link_libraries(decomposer treeDAG ${Boost_LIBRARIES})
