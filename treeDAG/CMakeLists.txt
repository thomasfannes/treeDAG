set(SOURCES
    separatorConfig.hpp
    separation.hpp
    separation.hxx
    separator.hpp
    separator.hxx

    separatorCache.hpp
    separatorCache.hxx

    treeDAG.cpp

    treeDecompositionDAG.hpp
    treeDecompositionDAG.cpp
    treeDecompositionDAG.hxx
    treeDecompositionDAGAndNode.hpp
    treeDecompositionDAGAndNode.cpp

    treeDecompositionSubgraph.hpp
    treeDecompositionSubgraph.cpp

    decomposer.hpp
    decomposer.hxx
    decompositionDAG.hpp
    decompositionDAG.hxx


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

add_library(treeDAG ${SOURCES})

