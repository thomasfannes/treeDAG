#ifndef TREEDAG_DECOMPOSITIONDAG_HXX
#define TREEDAG_DECOMPOSITIONDAG_HXX

#include "decompositionDAG.hpp"

namespace treeDAG {


template <typename SubgraphNodeDataIterator>
DecompositionDAG::NodeDescriptor DecompositionDAG::addSeparator(const SeparatorNodeData & separatorNodeData, SubgraphNodeDataIterator first, SubgraphNodeDataIterator last)
{
    // do we already have the separator node
    NodeDescriptor sepNode = findSeparatorNode(separatorNodeData);
    if(sepNode != InvalidNode() && boost::out_degree(sepNode, dag_) != 0)
        throw std::logic_error("DecompositionDAG: The separator node has already been processed");

    // should we create the node?
    if(sepNode == InvalidNode())
    {
        // yes, create the node and store in the map
        sepNode = boost::add_vertex(NODE_Separator, dag_);
        separatorMap_.left.insert(std::make_pair(sepNode, separatorNodeData));
    }

    // add the out edges
    for(; first != last; ++first)
        boost::add_edge(sepNode, findOrCreateSubgraphNode(*first), dag_);

    return sepNode;
}

template <typename SeparatorNodeIterator>
void DecompositionDAG::addClique(NodeDescriptor subgraphNode, const VertexSet & clique, SeparatorNodeIterator first, SeparatorNodeIterator last)
{
    assert(nodeType(subgraphNode) == NODE_Subgraph);
    NodeDescriptor cliqueNode = boost::add_vertex(NODE_Clique, dag_);
    cliqueMap_.insert(std::make_pair(cliqueNode, clique));

    // add the edges
    boost::add_edge(subgraphNode, cliqueNode, dag_);

    for(; first != last; ++first)
    {
        assert(nodeType(*first) == NODE_Separator);
        boost::add_edge(cliqueNode, *first, dag_);
    }
}

} // treeDAG namespace


#endif // TREEDAG_DECOMPOSITIONDAG_HXX
