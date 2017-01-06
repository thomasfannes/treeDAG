#include "separator.hpp"
#include <stack>

namespace treeDAG {


Separator::Separator(const Graph * graph)
    : graph_(graph)
{
}


std::size_t Separator::separateIntoComponentMap(const VertexSet & separator, ComponentMap & componentMap) const
{
    // make sure the component are the right size
    assert(componentMap == std::vector<VertexIndexType>(boost::num_vertices(*graph_), UnassignedVertex()));

    // set the separator
    for(VertexSet::const_iterator it = separator.begin(); it != separator.end(); ++it)
        componentMap[*it] = SeparatorVertex();

    // get and iterator over the vertices, and go to the first non-separator vertex
    std::pair<VertexIterator, VertexIterator> p = boost::vertices(*graph_);
    p.first = findFirstNonSeparatorVertex(p.first, p.second, componentMap);

    std::size_t noComponents = 0;

    while(p.first != p.second)
    {
        // fill the component
        fillCurrentComponent(*p.first, componentMap, noComponents);

        // find the next iterator
        p.first = findFirstNonSeparatorVertex(++p.first, p.second, componentMap);

        // and next color
        ++noComponents;
    }

    return noComponents;
}


void Separator::fillComponents(const ComponentMap & componentMap, ComponentSet & components) const
{
    // loop over the componentMap
    for(VertexIndexType curV = 0; curV < componentMap.size(); ++curV)
    {
        // get the current component number for this vertex
        VertexIndexType curComp = componentMap[curV];
        assert(curComp != UnassignedVertex());

        // a separator? then check the neighbours
        if(curComp == SeparatorVertex())
        {
            typedef boost::graph_traits<Graph>::adjacency_iterator adjIt;
            for(std::pair<adjIt,adjIt> p = boost::adjacent_vertices(curV, *graph_);p.first != p.second; ++p.first)
            {
                // get the necessary information for the adjacent vertex
                VertexIndexType adjV = *p.first;
                VertexIndexType adjComp = componentMap[adjV];

                // do not add separator vertices
                if(adjComp == SeparatorVertex())
                    continue;

                // did we already add this separator vertex?
                if(components[adjComp].empty() || components[adjComp].back() != curV)
                    components[adjComp].push_back(curV);
            }
        }
        // a normal vertex, just add to the right component
        else
        {
            components[curComp].push_back(curV);
        }
    }
}


Separator::VertexIterator
Separator::findFirstNonSeparatorVertex(VertexIterator current, VertexIterator last, const ComponentMap &componentMap) const
{
    for(; current != last; ++current)
    {
        if(componentMap[*current] == UnassignedVertex())
            return current;
    }
    return last;
}




void Separator::fillCurrentComponent(VertexIndexType source, ComponentMap & componentMap, std::size_t componentNumber) const
{
    typedef boost::graph_traits<Graph>::adjacency_iterator adjIt;

    std::stack<VertexIndexType> todo;
    todo.push(source);

    while(!todo.empty())
    {
        // get the next vertex to process
        VertexIndexType curV = todo.top();
        todo.pop();

        // get the color
        VertexIndexType & curColor = componentMap[curV];
        assert(curColor == UnassignedVertex() || curColor == SeparatorVertex() || curColor == componentNumber);

        // unseen vertex
        if(curColor == UnassignedVertex())
        {
            // set the color
            curColor = componentNumber;

            // add the neighbours to the todo
            for(std::pair<adjIt, adjIt> p = boost::adjacent_vertices(curV, *graph_); p.first != p.second; ++p.first)
                todo.push(*p.first);
        }
    }
}





} // treeDAG namespace
