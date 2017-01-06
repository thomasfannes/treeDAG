set(SOURCES
    separatorConfig.hpp

    separation.hpp
    separation.cpp
    separator.hpp
    separator.cpp
    separator.hxx

    separatorCache.hpp
    separatorCache.cpp

    decompositionDAG.hpp
    decompositionDAG.cpp
    decomposer.hpp
    decomposer.hxx
    decomposer.cpp

    treeDAG.cpp



    treeDecompositionDAG.hpp
    treeDecompositionDAG.cpp
    treeDecompositionDAG.hxx
    treeDecompositionDAGAndNode.hpp
    treeDecompositionDAGAndNode.cpp

    treeDecompositionSubgraph.hpp
    treeDecompositionSubgraph.cpp





  #separator.hpp
  #separator.cpp
  #separatorIterator.hpp
  #separatorIterator.cpp
  #separateUtil.hpp
  #separateUtil.cpp
  #separatorDAG.hpp
  #separatorDAG.cpp
  #treeDecompositionSubgraph.hpp
  #treeDecompositionSubgraph.cpp
  #dot.hpp
  #dot.cpp

  #separatorDAGConstructor.hpp
  #separatorDAGConstructor.cpp

  #detail/config.hpp
  #detail/separatorConfig.hpp
  util/kOrderedPermutateIterator.hpp
  util/tvsArray.hpp
  util/tvsArray.hxx

  #detail/entityWorkerGraph.hpp
  #detail/entityWorkerGraph.hxx
  #detail/entityWorkerGraphConfig.hpp

  #detail/separatorDAGConfig.hpp
  #detail/separatorDAGConfig.cpp
)

find_package(Boost 1.53 COMPONENTS system REQUIRED)

add_library(treeDAG ${SOURCES})
target_link_libraries(treeDAG ${Boost_LIBRARIES})

