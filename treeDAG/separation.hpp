#ifndef TREEDAG_SEPARATION_HPP
#define TREEDAG_SEPARATION_HPP

#include <limits>
#include <vector>

#include "separatorConfig.hpp"

namespace treeDAG {

template <typename _VertexIndexType = SeparatorConfig::VertexIndexType>
struct Separation
{
    typedef _VertexIndexType VertexIndexType;

    static VertexIndexType unassignedVertex() { return std::numeric_limits<VertexIndexType>::max(); }
    static VertexIndexType separatorVertex() { return std::numeric_limits<VertexIndexType>::max() - 1; }

    explicit Separation(VertexIndexType noVertices = 0)
        : noComponents(0),
          componentMap(noVertices, unassignedVertex())
    {
    }

    VertexIndexType noComponents;
    std::vector<VertexIndexType> componentMap;
};




} // namespace treeDAG

#endif // TREEDAG_SEPARATION_HPP
