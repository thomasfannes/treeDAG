#ifndef TREEDAG_DECOMPOSITIONDAG_HPP
#define TREEDAG_DECOMPOSITIONDAG_HPP

#include "separatorConfig.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/unordered_map.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

#include "separation.hpp"

namespace treeDAG {

struct SeparatorNode : public SeparatorConfig
{
    static VertexIndexType NoInactiveComponentNumber() { return std::numeric_limits<VertexIndexType>::max(); }

    VertexSet separator;
    std::size_t inactiveComponent;
};

struct SubgraphNode : public SeparatorConfig
{
    VertexSet activeVertices;
    VertexSet otherVertices;
};

std::size_t hash_value(const SeparatorNode & separatorNode);
std::size_t hash_value(const SubgraphNode & subgraphNode);

bool operator==(const SeparatorNode & lhs, const SeparatorNode & rhs);
bool operator==(const SubgraphNode & lhs, const SubgraphNode & rhs);

std::ostream & operator<<(std::ostream & str, const SeparatorNode & separatorNode);
std::ostream & operator<<(std::ostream & str, const SubgraphNode & separatorNode);



class DecompositionDAG : public boost::noncopyable, public SeparatorConfig
{
    typedef boost::adjacency_list<boost::listS, boost::listS, boost::bidirectionalS>                                        Structure;
    typedef boost::graph_traits<Structure>::vertex_descriptor                                                               NodeDescriptor;
    typedef boost::bimap<boost::bimaps::unordered_set_of<NodeDescriptor>, boost::bimaps::unordered_set_of<SubgraphNode > >  SubgraphMap;
    typedef boost::bimap<boost::bimaps::unordered_set_of<NodeDescriptor>, boost::bimaps::unordered_set_of<SeparatorNode > > SeparatorMap;

public:
    void addSeparatorNodes(const Separation & separation);
    void write_dot(std::ostream & stream) const;


private:
    void writeVertexSet(std::ostream & stream, const VertexSet & vertexSet) const;

    NodeDescriptor intAddSeparatorNode(const SeparatorNode & separatorNode);
    NodeDescriptor intAddSubgraphNode(const SubgraphNode & subgraphNode);
    bool hasSeparatorNode(const SeparatorNode & separatorNode) const;
    bool hasSubgraphNode(const SubgraphNode & subgraphNode) const;



    Structure dag_;
    boost::unordered_map<NodeDescriptor, VertexSet> cliqueMap_;
    SubgraphMap subgraphMap_;
    SeparatorMap separatorMap_;

};

} // namespace treeDAG

#endif // TREEDAG_DECOMPOSITIONDAG_HPP
