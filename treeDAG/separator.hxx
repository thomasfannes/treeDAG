#ifndef TREEDAG_SEPARATOR_HXX
#define TREEDAG_SEPARATOR_HXX

#include "separator.hpp"
#include <stack>

namespace treeDAG {

template <typename SeparatorVertexIterator>
void Separator::separate(SeparatorVertexIterator first, SeparatorVertexIterator last, result_type & separation) const
{   
    // set the separator
    std::set<VertexIndexType> separator(first, last);
    separation.separator.assign(separator.begin(), separator.end());

    // reserve for the component map
    separation.componentMap.assign(boost::num_vertices(*graph_), UnassignedVertex());

    // separate into the map
    std::size_t noComponents = separateIntoComponentMap(separation.separator, separation.componentMap);

    // create the space for the components
    separation.components.assign(noComponents, VertexSet());

    // and now extract the different components (together with the adjacent separator vertices)
    fillComponents(separation.componentMap, separation.components);
}

template <typename SeparatorVertexIterator>
Separator::result_type
Separator::operator()(SeparatorVertexIterator first, SeparatorVertexIterator last) const
{
    result_type separation;
    separate(first, last, separation);

    return separation;
}

template <typename SeparatorVertexIterator>
Separator::result_type
Separator::operator()(const std::pair<SeparatorVertexIterator, SeparatorVertexIterator> & separatorRange) const
{
    return operator ()(separatorRange.first, separatorRange.second);
}

} // treeDAG namespace

#endif // TREEDAG_SEPARATOR_HXX
