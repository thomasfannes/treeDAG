#ifndef TREEDAG_SEPARATORCONFIG_HPP
#define TREEDAG_SEPARATORCONFIG_HPP

#include <boost/graph/adjacency_list.hpp>

namespace treeDAG {

struct SeparatorConfig
{
    typedef unsigned char                                                           VertexIndexType;
    typedef boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS>    Graph;
};

} // treeDAG namespace

#endif // TREEDAG_SEPARATORCONFIG_HPP
