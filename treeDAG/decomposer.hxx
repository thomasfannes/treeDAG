#ifndef TREEDAG_DECOMPOSER_HXX
#define TREEDAG_DECOMPOSER_HXX

#include "decomposer.hpp"
#include "util/kOrderedPermutateIterator.hpp"
#include "util/combination.hpp"
#include <iostream>


namespace treeDAG {

template <typename VertexIterator>
void Decomposer::process(VertexIterator firstRoot, VertexIterator lastRoot)
{
    // start by setting the roots
    std::set<VertexIndexType> roots(firstRoot, lastRoot);
    roots_.assign(roots.begin(), roots.end());

    // create the root assignment
    SubgraphNodeData data;
    std::size_t graphSize = boost::num_vertices(*graph_);
    VertexSet::const_iterator sepIt = roots_.begin();

    // loop over all vertices
    for(std::size_t curV = 0; curV < graphSize; ++curV)
    {
        // distribute between other and active vertices
        if(sepIt == roots_.end() || *sepIt > curV)
            data.otherVertices.push_back(curV);
        else
        {
            data.activeVertices.push_back(*sepIt);
            ++sepIt;
        }
    }

    // create the subgraph node
    DecompositionDAG::NodeDescriptor nd = dag_.addSubgraph(data);
    todo_.push(nd);

    while(!todo_.empty())
    {
        DecompositionDAG::NodeDescriptor nd = todo_.top();
        todo_.pop();

        // already processed
        if(!processed_.insert(nd).second)
            continue;

        assert(dag_.nodeType(nd) == DecompositionDAG::NODE_Subgraph);

        process(nd);
    }

    dag_.cleanUp();
}

} // namespace treeDAG

#endif // TREEDAG_DECOMPOSER_HXX
