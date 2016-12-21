#ifndef TREEDAG_SEPARATOR_HXX
#define TREEDAG_SEPARATOR_HXX

#include "separator.hpp"
#include <stack>

namespace treeDAG {

#define TDEF template <typename _VertexIndexType>
#define CDEF Separator<_VertexIndexType>

TDEF
CDEF::Separator(const Graph * graph)
    : graph_(graph)
{
}

TDEF
template <typename SeparatorVertexIterator>
void CDEF::separate(SeparatorVertexIterator first, SeparatorVertexIterator last, result_type & separation) const
{
    // make sure the size is reserved
    assert(separation.componentMap.size() == boost::num_vertices(*graph_));
    assert(static_cast<std::size_t>(std::count(separation.componentMap.begin(), separation.componentMap.end(), separation.unassignedVertex())) == boost::num_vertices(*graph_));
    assert(separation.noComponents == 0);

    // set the separator
    for(; first != last; ++first)
        separation.componentMap[*first] = separation.separatorVertex();

    // get and iterator over the vertices, and go to the first non-separator vertex
    std::pair<VertexIterator, VertexIterator> p = boost::vertices(*graph_);
    p.first = findFirstNonSeparatorVertex(p.first, p.second, separation);

    while(p.first != p.second)
    {
        // fill the component
        fillCurrentComponent(*p.first, separation);

        // find the next iterator
        p.first = findFirstNonSeparatorVertex(++p.first, p.second, separation);

        // and next color
        ++separation.noComponents;
    }
}


TDEF
template <typename SeparatorVertexIterator>
typename CDEF::result_type
CDEF::operator()(SeparatorVertexIterator first, SeparatorVertexIterator last) const
{
    result_type separation(boost::num_vertices(*graph_));
    separate(first, last, separation);

    return separation;
}

TDEF
template <typename SeparatorVertexIterator>
typename CDEF::result_type
CDEF::operator()(const std::pair<SeparatorVertexIterator, SeparatorVertexIterator> & separatorRange) const
{
    return operator ()(separatorRange.first, separatorRange.second);
}

TDEF
typename CDEF::VertexIterator
CDEF::findFirstNonSeparatorVertex(VertexIterator current, VertexIterator last, const result_type & result) const
{
    const std::vector<VertexIndexType> & componentMap = result.componentMap;

    for(; current != last; ++current)
    {
        if(componentMap[*current] == result.unassignedVertex())
            return current;
    }
    return last;
}

TDEF
template <typename SeparatorVertexIterator>
std::list<typename CDEF::VertexSet> CDEF::findMaximalComponents(SeparatorVertexIterator first, SeparatorVertexIterator last) const
{
    typedef boost::graph_traits<Graph>::adjacency_iterator adjIt;

    // first separate everything
    Separation<VertexIndexType> separation = this->operator ()(first, last);

    // extract the separator
    std::set<VertexIndexType> separatorVertices(first, last);

    std::list<VertexSet> result;

    // now we loop over all components
    for(std::size_t curComponent = 0; curComponent < separation.noComponents; ++curComponent)
    {
        VertexSet component;

        // extract the component
        for(std::size_t i = 0; i < separation.componentMap.size(); ++i)
            if(separation.componentMap[i] == curComponent)
                component.push_back(i);

        // and no find the neighboring separators
        std::set<VertexIndexType> neighborhood;
        for(typename VertexSet::const_iterator it = component.begin(); it != component.end(); ++it)
            for(std::pair<adjIt,adjIt> p =boost::adjacent_vertices(*it, *graph_); p.first != p.second; ++p.first)
                if(separation.componentMap[*p.first] == separation.separatorVertex())
                    neighborhood.insert(*p.first);

        // all separator vertices found?
        if(neighborhood != separatorVertices)
            continue;

        // add the separator vertices
        component.insert(component.end(), separatorVertices.begin(), separatorVertices.end());
        std::sort(component.begin(), component.end());

        // and store
        result.push_back(component);
    }

    return result;
}

TDEF
void CDEF::fillCurrentComponent(VertexIndexType source, result_type & result) const
{
    typedef boost::graph_traits<Graph>::adjacency_iterator adjIt;

    const VertexIndexType & componentNumber = result.noComponents;
    std::vector<VertexIndexType> & componentMap = result.componentMap;

    std::stack<VertexIndexType> todo;
    todo.push(source);

    while(!todo.empty())
    {
        // get the next vertex to process
        VertexIndexType curV = todo.top();
        todo.pop();

        // get the color
        VertexIndexType & curColor = componentMap[curV];
        assert(curColor == result.unassignedVertex() || curColor == result.separatorVertex() || curColor == componentNumber);

        // unseen vertex
        if(curColor == result.unassignedVertex())
        {
            // set the color
            curColor = componentNumber;

            // add the neighbours to the todo
            for(std::pair<adjIt, adjIt> p = boost::adjacent_vertices(curV, *graph_); p.first != p.second; ++p.first)
                todo.push(*p.first);
        }
    }
}

#undef TDEF
#undef CDEF

} // treeDAG namespace

#endif // TREEDAG_SEPARATOR_HXX
