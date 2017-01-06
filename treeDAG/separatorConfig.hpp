#ifndef TREEDAG_SEPARATORCONFIG_HPP
#define TREEDAG_SEPARATORCONFIG_HPP

#include <boost/graph/adjacency_list.hpp>

namespace treeDAG {

struct SeparatorConfig
{
    typedef boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS>    Graph;
    typedef std::size_t                                                             VertexIndexType;
    typedef std::vector<VertexIndexType>                                            VertexSet;
    typedef std::vector<VertexSet>                                                  ComponentSet;
    typedef std::vector<VertexIndexType>                                            ComponentMap;

    static VertexIndexType UnassignedVertex() { return std::numeric_limits<VertexIndexType>::max(); }
    static VertexIndexType SeparatorVertex() { return std::numeric_limits<VertexIndexType>::max() - 1; }
};

} // treeDAG namespace

#endif // TREEDAG_SEPARATORCONFIG_HPP
