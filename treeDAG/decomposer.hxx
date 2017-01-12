#ifndef TREEDAG_DECOMPOSER_HXX
#define TREEDAG_DECOMPOSER_HXX

#include "decomposer.hpp"


namespace treeDAG {

template <typename VertexIterator>
void Decomposer::process(VertexIterator firstRoot, VertexIterator lastRoot)
{
    // start by setting the roots
    std::set<VertexIndexType> roots(firstRoot, lastRoot);
    roots_.assign(roots.begin(), roots.end());

    processRoot();

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
