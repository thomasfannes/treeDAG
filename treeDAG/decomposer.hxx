#ifndef TREEDAG_DECOMPOSER_HXX
#define TREEDAG_DECOMPOSER_HXX

#include "decomposer.hpp"
#include "util/kOrderedPermutateIterator.hpp"
#include "util/combination.hpp"
#include <iostream>


namespace treeDAG {

template <typename VertexIterator>
std::size_t Decomposer::process(VertexIterator firstRoot, VertexIterator lastRoot)
{
    // store the roots
    roots_.assign(firstRoot, lastRoot);
    std::sort(roots_.begin(), roots_.end());

    // add all the separators to the dag
    for(std::pair<SeparatorCache::SeparatorIterator, SeparatorCache::SeparatorIterator> p = cache_.separators(); p.first != p.second; ++p.first)
        dag_.addSeparatorNodes(*p.first);



//    // create the root requirement
//    TreeDecompositionSubgraph root(boost::num_vertices(*graph_));
//    for(std::size_t i = 0; i < root.size(); ++i)
//        root[i] = TreeDecompositionSubgraph::ProjectedAwayVertex;
//    for(; firstRoot != lastRoot; ++firstRoot)
//        root[*firstRoot] = TreeDecompositionSubgraph::ActiveVertex;

//    // add to the list
//    todo_.push(root);

//    // and process the complete todo set
//    while(!todo_.empty())
//    {
//        // extract the top element
//        TreeDecompositionSubgraph cur = todo_.top();
//        todo_.pop();

//        // already handled?
//        if(!processed_.insert(cur).second)
//            continue;

//        // add to the dag
//        dag_.addNode(cur);

//        // process it
//        processSubgraph(cur);
//    }

//    return processed_.size();

    return 0;
}

} // namespace treeDAG

#endif // TREEDAG_DECOMPOSER_HXX
