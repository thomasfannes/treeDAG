set (Boost_USE_MULTITHREADED OFF)
find_package(Boost 1.53 COMPONENTS unit_test_framework REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})

add_library(util util.cpp)

add_executable(separatorTest separatorTest.cpp)
target_link_libraries(separatorTest util treeDAG ${Boost_LIBRARIES})
add_test(NAME separatorTest COMMAND separatorTest)

#add_executable(separatorIteratorTest separatorIteratorTest.cpp)
#target_link_libraries(separatorIteratorTest util treeDAG ${Boost_LIBRARIES})
#add_test(NAME separatorIteratorTest COMMAND separatorIteratorTest)

#add_executable(separateUtilTest separateUtilTest.cpp)
#target_link_libraries(separateUtilTest util treeDAG ${Boost_LIBRARIES})
#add_test(NAME separateUtilTest COMMAND separateUtilTest)

#add_executable(separatorDAGTest separatorDAGTest.cpp)
#target_link_libraries(separatorDAGTest util treeDAG ${Boost_LIBRARIES})
#add_test(NAME separatorDAGTest COMMAND separatorDAGTest)

#add_executable(separatorDAGConstructorTest separatorDAGConstructorTest.cpp)
#target_link_libraries(separatorDAGConstructorTest util treeDAG ${Boost_LIBRARIES})
#add_test(NAME separatorDAGConstructorTest COMMAND separatorDAGConstructorTest)



