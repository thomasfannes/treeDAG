#ifndef TREEDAG_DECOMPOSITIONDAG_HPP
#define TREEDAG_DECOMPOSITIONDAG_HPP

#include "separatorConfig.hpp"
#include "separation.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/unordered_map.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>



namespace treeDAG {

struct SeparatorNodeData : public SeparatorConfig
{
    static VertexIndexType NoInactiveComponentNumber() { return std::numeric_limits<VertexIndexType>::max(); }

    VertexSet separator;
    VertexSet inactiveComponents;
};

struct SubgraphNodeData : public SeparatorConfig
{
    VertexSet activeVertices;
    VertexSet otherVertices;
};

std::size_t hash_value(const SeparatorNodeData & separatorNode);
std::size_t hash_value(const SubgraphNodeData & subgraphNode);

bool operator==(const SeparatorNodeData & lhs, const SeparatorNodeData & rhs);
bool operator==(const SubgraphNodeData & lhs, const SubgraphNodeData & rhs);

std::ostream & operator<<(std::ostream & str, const SeparatorNodeData & separatorNode);
std::ostream & operator<<(std::ostream & str, const SubgraphNodeData & separatorNode);



class DecompositionDAG : public boost::noncopyable, public SeparatorConfig
{
public:
    enum NodeType
    {
        NODE_Separator,
        NODE_Subgraph,
        NODE_Clique

    };

    typedef boost::adjacency_list<boost::listS, boost::listS, boost::bidirectionalS, NodeType>                              Structure;
    typedef boost::graph_traits<Structure>::vertex_descriptor                                                               NodeDescriptor;

    // output methods
    void write_dot(std::ostream & stream) const;

    // structure methods
    const Structure & structure() const;
    NodeType nodeType(NodeDescriptor node) const;

    // find methods
    NodeDescriptor findSeparatorNode(const SeparatorNodeData & separatorNodeData) const;
    NodeDescriptor findSubgraphNode(const SubgraphNodeData & subgraphNodeData) const;
    const SeparatorNodeData * separatorNodeData(NodeDescriptor separatorNode) const;
    const SubgraphNodeData * subgraphNodeData(NodeDescriptor subgraphNode) const;

    // addition methods
    template <typename SubgraphNodeDataIterator>
    NodeDescriptor addSeparator(const SeparatorNodeData & separatorNodeData, SubgraphNodeDataIterator first, SubgraphNodeDataIterator last);
    NodeDescriptor addSubgraph(const SubgraphNodeData & subgraphNodeData);

    template <typename SeparatorNodeIterator>
    void addClique(NodeDescriptor subgraphNode, const VertexSet & clique, SeparatorNodeIterator first, SeparatorNodeIterator last);

    void cleanUp();

    std::size_t numberOfNodes() const { return boost::num_vertices(dag_); }
    std::size_t numberOfBranches() const { return boost::num_edges(dag_); }



    static NodeDescriptor InvalidNode() { return boost::graph_traits<Structure>::null_vertex(); }


private:
    typedef boost::bimap<boost::bimaps::unordered_set_of<NodeDescriptor>, boost::bimaps::unordered_set_of<SubgraphNodeData > >  SubgraphMap;
    typedef boost::bimap<boost::bimaps::unordered_set_of<NodeDescriptor>, boost::bimaps::unordered_set_of<SeparatorNodeData > > SeparatorMap;
    typedef boost::unordered_map<NodeDescriptor, VertexSet>                                                                     CliqueSizeMap;

    void writeVertexSet(std::ostream & stream, const VertexSet & vertexSet) const;

    NodeDescriptor findOrCreateSeparatorNode(const SeparatorNodeData & separatorNode);
    NodeDescriptor findOrCreateSubgraphNode(const SubgraphNodeData & subgraphNode);

    void checkCliqueNode(NodeDescriptor node, boost::unordered_map<NodeDescriptor, std::size_t> & countMap);
    void checkSeparatorNode(NodeDescriptor node, boost::unordered_map<NodeDescriptor, std::size_t> & countMap);
    void checkSubgraphNode(NodeDescriptor node, boost::unordered_map<NodeDescriptor, std::size_t> & countMap);
    void cleanSubtree(NodeDescriptor node, boost::unordered_map<NodeDescriptor, std::size_t> & countMap);


    Structure dag_;
    SubgraphMap subgraphMap_;
    SeparatorMap separatorMap_;
    CliqueSizeMap cliqueMap_;
};


} // namespace treeDAG

#include "decompositionDAG.hxx"

#endif // TREEDAG_DECOMPOSITIONDAG_HPP
