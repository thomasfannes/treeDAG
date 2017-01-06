#include "decompositionDAG.hpp"

namespace treeDAG {
namespace {

template <typename Iterator>
struct IteratorStreamer
{
    explicit IteratorStreamer(const char * delim = ",") : delim_(delim) { }
    IteratorStreamer(Iterator first, Iterator last, const char * delim = ",")
        : range_(first, last),
          delim_(delim)
    {
    }

    void operator()(std::ostream & stream) const
    {
        for(Iterator it = range_.first; it != range_.second; ++it)
            stream << (it != range_.first ? delim_ : "") << *it;
    }

private:
    std::pair<Iterator, Iterator> range_;
    const char * delim_;
};

template <typename Iterator>
std::ostream & operator<<(std::ostream & stream, const IteratorStreamer<Iterator> & streamer)
{
    streamer(stream);
    return stream;
}
template <typename Iterator>
IteratorStreamer<Iterator> make_streamer(Iterator first, Iterator last, const char * delim = ",")
{
    return IteratorStreamer<Iterator>(first, last, delim);
}

template <typename T, typename Alloc>
IteratorStreamer<typename std::vector<T, Alloc>::const_iterator> make_streamer(const std::vector<T, Alloc> & vct, const char * delim = ",")
{
    return IteratorStreamer<typename std::vector<T, Alloc>::const_iterator>(vct.begin(), vct.end(), delim);
}

} //  namespace



std::size_t hash_value(const SeparatorNode & separatorNode)
{
    std::size_t seed = boost::hash_value(separatorNode.inactiveComponent);
    boost::hash_combine(seed, separatorNode.separator);

    return seed;
}


std::size_t hash_value(const SubgraphNode & subgraphNode)
{
    std::size_t seed = boost::hash_value(subgraphNode.activeVertices);
    boost::hash_combine(seed, subgraphNode.activeVertices);

    return seed;
}


bool operator==(const SeparatorNode & lhs, const SeparatorNode & rhs)
{
    return lhs.inactiveComponent == rhs.inactiveComponent && lhs.separator == rhs.separator;
}


bool operator==(const SubgraphNode & lhs, const SubgraphNode & rhs)
{
    return lhs.activeVertices == rhs.activeVertices && lhs.otherVertices == rhs.otherVertices;
}


std::ostream & operator<<(std::ostream & str, const SeparatorNode & separatorNode)
{
    str << "S(" << make_streamer(separatorNode.separator, ",") << ")";
    if(separatorNode.inactiveComponent != separatorNode.NoInactiveComponentNumber())
        str << " [" << separatorNode.inactiveComponent << "]";
    return str;
}


std::ostream & operator<<(std::ostream & str, const SubgraphNode & subgraphNode)
{
    str << "G(" << make_streamer(subgraphNode.activeVertices, ", ") << "),*(" << make_streamer(subgraphNode.otherVertices, ", ") << ")";
    return str;
}

void DecompositionDAG::addSeparatorNodes(const Separation & separation)
{
    const VertexSet & separator = separation.separator;
    const ComponentSet & components = separation.components;


    // reserve space for storing all the subgraph nodes
    std::vector<NodeDescriptor> compNodes(components.size(), boost::graph_traits<Structure>::null_vertex());

    // first add all the components
    for(std::size_t i = 0; i < components.size(); ++i)
    {
        const VertexSet & curComp = components[i];

        // create a new subgraph node
        SubgraphNode subgraphNode;
        subgraphNode.activeVertices = separator;

        // set the other vertices
        for(typename VertexSet::const_iterator it = curComp.begin(); it != curComp.end(); ++it)
            // is it a separator node?
            if(std::find(separator.begin(), separator.end(), *it) == separator.end())
                subgraphNode.otherVertices.push_back(*it);

        // now create the actual node
        compNodes[i] = intAddSubgraphNode(subgraphNode);
    }

    // add a separator with everything active
    {
        SeparatorNode sepNode;
        sepNode.separator = separator;
        sepNode.inactiveComponent = sepNode.NoInactiveComponentNumber();

        // do not add it double, especially not the edges
        if(!hasSeparatorNode(sepNode))
        {
            NodeDescriptor nd = intAddSeparatorNode(sepNode);
            for(std::size_t i = 0; i < compNodes.size(); ++i)
                boost::add_edge(nd, compNodes[i], dag_);
        }
    }

    // and now add separator nodes for one-out
    for(std::size_t ignored = 0; ignored != compNodes.size(); ++ignored)
    {
        SeparatorNode sepNode;
        sepNode.separator = separator;
        sepNode.inactiveComponent = ignored;

        // do not add it double, especially not the edges
        if(!hasSeparatorNode(sepNode))
        {
            NodeDescriptor nd = intAddSeparatorNode(sepNode);
            for(std::size_t i = 0; i < compNodes.size(); ++i)
                if(i != ignored)
                    boost::add_edge(nd, compNodes[i], dag_);
        }
    }

}


void DecompositionDAG::write_dot(std::ostream & stream) const
{
    stream << "digraph G {" << std::endl;

    std::size_t curIndex = 0;
    boost::unordered_map<NodeDescriptor, std::size_t> subgraphNodeIndexMap;

    // write the subgraph nodes
    for(typename SubgraphMap::right_const_iterator it = subgraphMap_.right.begin(); it != subgraphMap_.right.end(); ++it, ++curIndex)
    {
        const SubgraphNode & subgraph = it->first;
        NodeDescriptor node = it->second;

        // write it out
        stream << "  n" << curIndex << " [label=\"" << subgraph << "\"];" << std::endl;

        // store the index
        subgraphNodeIndexMap.insert(std::make_pair(node, curIndex));
    }

    // reset the index
    curIndex = 0;
    boost::unordered_map<NodeDescriptor, std::size_t> separatorNodeIndexMap;
    // write the subgraph nodes
    for(typename SeparatorMap::right_const_iterator it = separatorMap_.right.begin(); it != separatorMap_.right.end(); ++it, ++curIndex)
    {
        const SeparatorNode & separator = it->first;
        NodeDescriptor node = it->second;

        // write it out
        stream << "  s" << curIndex << " [label=\"" << separator << "\"];" << std::endl;

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
//        const VertexSet & clique = it->second;

        // write out the node
        stream << "  c" << curIndex << " [label=\"C\"];" << std::endl;

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


DecompositionDAG::NodeDescriptor DecompositionDAG::intAddSeparatorNode(const SeparatorNode & separatorNode)
{
    // already existing
    typename SeparatorMap::right_const_iterator it = separatorMap_.right.find(separatorNode);
    if(it != separatorMap_.right.end())
        return it->second;

    // add a new node
    NodeDescriptor nd = boost::add_vertex(dag_);
    separatorMap_.left.insert(std::make_pair(nd, separatorNode));

    return nd;
}


DecompositionDAG::NodeDescriptor DecompositionDAG::intAddSubgraphNode(const SubgraphNode & subgraphNode)
{
    // already existing
    typename SubgraphMap::right_const_iterator it = subgraphMap_.right.find(subgraphNode);
    if(it != subgraphMap_.right.end())
        return it->second;

    // add a new node
    NodeDescriptor nd = boost::add_vertex(dag_);
    subgraphMap_.left.insert(std::make_pair(nd, subgraphNode));

    return nd;
}


bool DecompositionDAG::hasSeparatorNode(const SeparatorNode & separatorNode) const
{
    return separatorMap_.right.find(separatorNode) != separatorMap_.right.end();
}


bool DecompositionDAG::hasSubgraphNode(const SubgraphNode & subgraphNode) const
{
    return subgraphMap_.right.find(subgraphNode) != subgraphMap_.right.end();
}


} // namespace treeDAG
