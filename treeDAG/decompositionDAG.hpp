#ifndef TREEDAG_DECOMPOSITIONDAG_HPP
#define TREEDAG_DECOMPOSITIONDAG_HPP

#include <boost/graph/adjacency_list.hpp>
#include <boost/unordered_map.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include "treeDecompositionSubgraph.hpp"

namespace treeDAG {

template <typename VertexIndexType>
class DecompositionDAG : public boost::noncopyable
{
    typedef boost::adjacency_list<boost::listS, boost::listS, boost::bidirectionalS> Structure;
    typedef boost::graph_traits<Structure>::vertex_descriptor NodeDescriptor;
    typedef std::vector<VertexIndexType> VertexSet;
    typedef boost::bimap<boost::bimaps::unordered_set_of<NodeDescriptor>, boost::bimaps::unordered_set_of<TreeDecompositionSubgraph> > SubgraphMap;
    typedef boost::bimap<boost::bimaps::unordered_set_of<NodeDescriptor>, boost::bimaps::unordered_set_of<VertexSet> > SeparatorMap;

public:
    void addNode(const TreeDecompositionSubgraph & subgraph);

    template <typename ChildIterator>
    void addClique(const TreeDecompositionSubgraph & parent, const VertexSet & clique, ChildIterator firstChild, ChildIterator lastChild);

    template <typename ChildIterator>
    void addSeparatorChildren(const VertexSet & separator, ChildIterator firstChild, ChildIterator lastChild);

    void write_dot(std::ostream & stream) const;


private:
    void writeVertexSet(std::ostream & stream, const VertexSet & vertexSet) const;

    NodeDescriptor intAddSubgraphNode(const TreeDecompositionSubgraph & subgraph);
    NodeDescriptor intAddSeparatorNode(const VertexSet & separator);


    Structure dag_;
    boost::unordered_map<NodeDescriptor, VertexSet> cliqueMap_;
    SubgraphMap subgraphMap_;
    SeparatorMap separatorMap_;

};

} // namespace treeDAG

#include "decompositionDAG.hxx"

#endif // TREEDAG_DECOMPOSITIONDAG_HPP
