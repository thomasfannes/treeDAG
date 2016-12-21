#ifndef TREEDAG_DECOMPOSITIONDAG_HXX
#define TREEDAG_DECOMPOSITIONDAG_HXX

#include "decompositionDAG.hpp"

namespace treeDAG {

#define TDEF template <typename VertexIndexType>
#define CDEF DecompositionDAG<VertexIndexType>

TDEF
void CDEF::addNode(const TreeDecompositionSubgraph & subgraph)
{
    intAddSubgraphNode(subgraph);
}

TDEF
template <typename ChildIterator>
void CDEF::addClique(const TreeDecompositionSubgraph & parent, const VertexSet & clique, ChildIterator firstChild, ChildIterator lastChild)
{
    // parent node
    NodeDescriptor pNode = intAddSubgraphNode(parent);

    // children node
    std::list<NodeDescriptor> cNodes;
    for(; firstChild != lastChild; ++firstChild)
        cNodes.push_back(intAddSeparatorNode(*firstChild));

    // create the clique node
    NodeDescriptor nd = boost::add_vertex(dag_);
    cliqueMap_.insert(std::make_pair(nd, clique));

    // and now the edges
    boost::add_edge(pNode, nd, dag_);
    for(typename std::list<NodeDescriptor>::const_iterator it = cNodes.begin(); it != cNodes.end(); ++it)
        boost::add_edge(nd, *it, dag_);
}

TDEF
template <typename ChildIterator>
void CDEF::addSeparatorChildren(const VertexSet & separator, ChildIterator firstChild, ChildIterator lastChild)
{
    // parent node
    NodeDescriptor pNode = intAddSeparatorNode(separator);

    // children node
    for(; firstChild != lastChild; ++firstChild)
        boost::add_edge(pNode, intAddSubgraphNode(*firstChild), dag_);
}

TDEF
void CDEF::writeVertexSet(std::ostream & stream, const VertexSet & vertexSet) const
{
    for(std::size_t i = 0; i < vertexSet.size(); ++i)
    {
        if(i != 0)
            stream << ",";
        stream << static_cast<std::size_t>(vertexSet[i]);
    }
}

TDEF
void CDEF::write_dot(std::ostream & stream) const
{
    stream << "digraph G {" << std::endl;

    std::size_t curIndex = 0;
    boost::unordered_map<NodeDescriptor, std::size_t> subgraphNodeIndexMap;

    // write the subgraph nodes
    for(typename SubgraphMap::right_const_iterator it = subgraphMap_.right.begin(); it != subgraphMap_.right.end(); ++it, ++curIndex)
    {
        const TreeDecompositionSubgraph & subgraph = it->first;
        NodeDescriptor node = it->second;

        // write it out
        stream << "  n" << curIndex << " [label=\"G" << subgraph << "\"];" << std::endl;

        // store the index
        subgraphNodeIndexMap.insert(std::make_pair(node, curIndex));
    }

    // reset the index
    curIndex = 0;
    boost::unordered_map<NodeDescriptor, std::size_t> separatorNodeIndexMap;
    // write the subgraph nodes
    for(typename SeparatorMap::right_const_iterator it = separatorMap_.right.begin(); it != separatorMap_.right.end(); ++it, ++curIndex)
    {
        const VertexSet & separator = it->first;
        NodeDescriptor node = it->second;

        // write it out
        stream << "  s" << curIndex << " [label=\"S{";
        writeVertexSet(stream, separator);
        stream << "}\"];" << std::endl;

        // store the index
        separatorNodeIndexMap.insert(std::make_pair(node, curIndex));

        // write the edges from separator node to subgraph node
        typedef boost::graph_traits<Structure>::adjacency_iterator adjIt;
        for(std::pair<adjIt, adjIt> p = boost::adjacent_vertices(node, dag_); p.first != p.second; ++p.first)
            stream << "  s" << curIndex << " -> n" << subgraphNodeIndexMap[*p.first] << ";" << std::endl;
    }

    // reset the index
    curIndex = 0;

    // and now the clique nodes (and the edges)
    for(typename boost::unordered_map<NodeDescriptor, VertexSet>::const_iterator it = cliqueMap_.begin(); it != cliqueMap_.end(); ++it, ++curIndex)
    {
        const VertexSet & clique = it->second;

        // write out the node
        stream << "  c" << curIndex << " [label=\"C{";
        writeVertexSet(stream, clique);
        stream << "}\"];" << std::endl;

        // and now write out the nodes
        typedef typename boost::graph_traits<Structure>::adjacency_iterator adjIt;
        for(std::pair<adjIt, adjIt> p = boost::adjacent_vertices(it->first, dag_); p.first != p.second; ++p.first)
            stream << "  c" << curIndex << " -> s" << separatorNodeIndexMap[*p.first] << ";" << std::endl;

        // and now the in nodes
        typedef typename Structure::inv_adjacency_iterator invAdjIt;
        for(std::pair<invAdjIt, invAdjIt> p = boost::inv_adjacent_vertices(it->first, dag_); p.first != p.second; ++p.first)
            stream << "  n" << subgraphNodeIndexMap[*p.first] << " -> c" << curIndex << ";" << std::endl;
    }

    stream << "}" << std::endl;
}


TDEF
typename CDEF::NodeDescriptor CDEF::intAddSubgraphNode(const TreeDecompositionSubgraph & subgraph)
{
    // already existing?
    typename SubgraphMap::right_const_iterator it = subgraphMap_.right.find(subgraph);
    if(it != subgraphMap_.right.end())
        return it->second;

    // add a new node
    NodeDescriptor nd = boost::add_vertex(dag_);
    subgraphMap_.left.insert(std::make_pair(nd, subgraph));

    return nd;
}

TDEF
typename CDEF::NodeDescriptor CDEF::intAddSeparatorNode(const VertexSet & separator)
{
    // already existing?
    typename SeparatorMap::right_const_iterator it = separatorMap_.right.find(separator);
    if(it != separatorMap_.right.end())
        return it->second;

    // add a new node
    NodeDescriptor nd = boost::add_vertex(dag_);
    separatorMap_.left.insert(std::make_pair(nd, separator));

    return nd;
}


#undef TDEF
#undef CDEF

} // namespace treeDAG

#endif // TREEDAG_DECOMPOSITIONDAG_HXX
