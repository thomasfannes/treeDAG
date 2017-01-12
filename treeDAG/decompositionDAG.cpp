#include "decompositionDAG.hpp"
#include <boost/graph/topological_sort.hpp>
#include <iostream>
#include <stack>

namespace treeDAG {
namespace {

typedef DecompositionDAG::VertexSet VertexSet;
bool isSubset(const VertexSet & possibleSubset, const VertexSet & possibleSuperset)
{
    VertexSet::const_iterator subIt = possibleSubset.begin();
    VertexSet::const_iterator supIt = possibleSuperset.begin();

    while(subIt != possibleSubset.end() && supIt != possibleSuperset.end())
    {
        // *subit does not occur in the superset
        if(*subIt < *supIt)
            return false;

        // if they are equal, we need to increase both
        if(*subIt == *supIt)
            ++subIt;

        // anyhow, increate the supper iterator
        ++supIt;
    }

    return subIt == possibleSubset.end();
}

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



std::size_t hash_value(const SeparatorNodeData & separatorNode)
{
    std::size_t seed = boost::hash_value(separatorNode.inactiveComponents);
    boost::hash_combine(seed, separatorNode.separator);

    return seed;
}


std::size_t hash_value(const SubgraphNodeData & subgraphNode)
{
    std::size_t seed = boost::hash_value(subgraphNode.activeVertices);
    boost::hash_combine(seed, subgraphNode.activeVertices);

    return seed;
}


bool operator==(const SeparatorNodeData & lhs, const SeparatorNodeData & rhs)
{
    return lhs.inactiveComponents == rhs.inactiveComponents && lhs.separator == rhs.separator;
}


bool operator==(const SubgraphNodeData & lhs, const SubgraphNodeData & rhs)
{
    return lhs.activeVertices == rhs.activeVertices && lhs.otherVertices == rhs.otherVertices;
}


std::ostream & operator<<(std::ostream & str, const SeparatorNodeData & separatorNode)
{
    str << "S(" << make_streamer(separatorNode.separator, ",") << ")";
    return str;
}


std::ostream & operator<<(std::ostream & str, const SubgraphNodeData & subgraphNode)
{
    str << "G(" << make_streamer(subgraphNode.activeVertices, ", ") << "),*(" << make_streamer(subgraphNode.otherVertices, ", ") << ")";
    return str;
}

const DecompositionDAG::Structure & DecompositionDAG::structure() const
{
    return dag_;
}

DecompositionDAG::NodeType DecompositionDAG::nodeType(NodeDescriptor node) const
{
    return dag_[node];
}

DecompositionDAG::NodeDescriptor DecompositionDAG::findSeparatorNode(const SeparatorNodeData & separatorNodeData) const
{
    SeparatorMap::right_const_iterator it = separatorMap_.right.find(separatorNodeData);
    if(it == separatorMap_.right.end())
        return InvalidNode();

    return it->second;
}

DecompositionDAG::NodeDescriptor DecompositionDAG::findSubgraphNode(const SubgraphNodeData & subgraphNodeData) const
{
    SubgraphMap::right_const_iterator it = subgraphMap_.right.find(subgraphNodeData);
    if(it == subgraphMap_.right.end())
        return InvalidNode();

    return it->second;
}
const SeparatorNodeData * DecompositionDAG::separatorNodeData(NodeDescriptor separatorNode) const
{
    assert(nodeType(separatorNode) == NODE_Separator);
    return &(separatorMap_.left.find(separatorNode)->second);
}

const SubgraphNodeData * DecompositionDAG::subgraphNodeData(NodeDescriptor subgraphNode) const
{
    assert(nodeType(subgraphNode) == NODE_Subgraph);
    return &(subgraphMap_.left.find(subgraphNode)->second);
}


DecompositionDAG::NodeDescriptor DecompositionDAG::addSubgraph(const SubgraphNodeData & subgraphNodeData)
{
    if(subgraphMap_.right.find(subgraphNodeData) != subgraphMap_.right.end())
        throw std::logic_error("DecompositionDAG: The subgraph node has already been processed");

    NodeDescriptor nd = boost::add_vertex(NODE_Subgraph, dag_);
    subgraphMap_.left.insert(std::make_pair(nd, subgraphNodeData));

    return nd;
}

void DecompositionDAGNodeStreamWriter::toStream(std::ostream & stream) const
{
    if(dag_ == 0)
        throw std::logic_error("DecompositionDAGNodeStreamWriter: initialized without a valid decomposition DAG object");

    const DecompositionDAG & dag = *dag_;

    switch (dag.nodeType(node_)) {
    case DecompositionDAG::NODE_Clique:
        stream << "C(" << make_streamer(dag.cliqueMap_.find(node_)->second) << ")";
        break;

    case DecompositionDAG::NODE_Separator:
        stream << dag.separatorMap_.left.find(node_)->second;
        break;

    case DecompositionDAG::NODE_Subgraph:
        stream << dag.subgraphMap_.left.find(node_)->second;
        break;
    }
}

std::ostream & operator<<(std::ostream & stream, const DecompositionDAGNodeStreamWriter & writer)
{
    writer.toStream(stream);
    return stream;
}

DecompositionDAGNodeStreamWriter DecompositionDAG::nodeWriter(NodeDescriptor node) const
{
    DecompositionDAGNodeStreamWriter w;
    w.node_ = node;
    w.dag_ = this;

    return w;
}

void DecompositionDAG::write_dot(std::ostream & stream) const
{
    typedef boost::graph_traits<Structure>::vertex_iterator vit;
    typedef boost::graph_traits<Structure>::edge_iterator   eit;

    stream << "digraph G {" << std::endl;

    // index for the index map
    std::size_t curIndex = 0;
    boost::unordered_map<NodeDescriptor, std::size_t> nodeIndexMap;



    // write out all the nodes and give them an id
    for(std::pair<vit, vit> p = boost::vertices(dag_); p.first != p.second; ++p.first)
    {
        NodeDescriptor node = *p.first;

        // start the line
        stream << "  v" << curIndex << " [label=\"" << nodeWriter(node) << "\"];" << std::endl;

        // end the line
        // store in the index map
        nodeIndexMap.insert(std::make_pair(node, curIndex++));
    }

    // write out all the edges
    for(std::pair<eit, eit> p =boost::edges(dag_); p.first != p.second; ++p.first)
    {
        NodeDescriptor src = boost::source(*p.first, dag_);
        NodeDescriptor tgt = boost::target(*p.first, dag_);

        // write the edge
        stream << "  v" << nodeIndexMap[src] << " -> v" << nodeIndexMap[tgt] << ";" << std::endl;
    }

    stream << "}" << std::endl;
}


DecompositionDAG::NodeDescriptor DecompositionDAG::findOrCreateSeparatorNode(const SeparatorNodeData & separatorNode)
{
    // already existing
    typename SeparatorMap::right_const_iterator it = separatorMap_.right.find(separatorNode);
    if(it != separatorMap_.right.end())
        return it->second;

    // add a new node
    NodeDescriptor nd = boost::add_vertex(NODE_Separator, dag_);
    separatorMap_.left.insert(std::make_pair(nd, separatorNode));

    return nd;
}


DecompositionDAG::NodeDescriptor DecompositionDAG::findOrCreateSubgraphNode(const SubgraphNodeData & subgraphNode)
{
    // already existing
    typename SubgraphMap::right_const_iterator it = subgraphMap_.right.find(subgraphNode);
    if(it != subgraphMap_.right.end())
        return it->second;

    // add a new node
    NodeDescriptor nd = boost::add_vertex(NODE_Subgraph, dag_);
    subgraphMap_.left.insert(std::make_pair(nd, subgraphNode));

    return nd;
}

void DecompositionDAG::cleanUp()
{
    typedef boost::graph_traits<Structure>::vertex_iterator vit;
    typedef boost::graph_traits<Structure>::adjacency_iterator adjIt;

    // start by creating a topological sort
    std::size_t curIndex = 0;
    boost::unordered_map<NodeDescriptor, std::size_t> indexMap;
    for(std::pair<vit, vit> p =boost::vertices(dag_); p.first != p.second; ++p.first)
        indexMap.insert(std::make_pair(*p.first, curIndex++));

    // sort it
    std::vector<NodeDescriptor> sorted(boost::num_vertices(dag_), InvalidNode());
    boost::topological_sort(dag_, sorted.begin(), boost::vertex_index_map(boost::make_assoc_property_map(indexMap)));

    // keep a count map
    boost::unordered_map<NodeDescriptor, std::size_t> countMap;



    // count is as follows:
    //  - for a subgraph the total number of vertices
    //  - for a separator the total number of vertices minus the separator
    //  - for a clique: the total number of vertices

    // loop over all nodes
    for(std::vector<NodeDescriptor>::const_iterator it = sorted.begin(); it != sorted.end(); ++it)
    {
        NodeDescriptor node = *it;
        switch(nodeType(node))
        {
        case NODE_Subgraph:
            checkSubgraphNode(node, countMap);
            break;
        case NODE_Separator:
            checkSeparatorNode(node, countMap);
            break;
        case NODE_Clique:
            checkCliqueNode(node, countMap);
            break;
        }
    }

    // and now clean the parallel edges
    cleanupParallelEdges();
}

void DecompositionDAG::checkSubgraphNode(NodeDescriptor node, boost::unordered_map<NodeDescriptor, std::size_t> & countMap)
{
    typedef boost::graph_traits<Structure>::adjacency_iterator adjIt;

    const SubgraphNodeData & data = *subgraphNodeData(node);
    std::size_t count = data.activeVertices.size() + data.otherVertices.size();
    countMap.insert(std::make_pair(node, count));

    std::list<NodeDescriptor> toRemove;

    // check whether we should remove any child clique nodes (because the count is of)
    for(std::pair<adjIt, adjIt> p = boost::adjacent_vertices(node, dag_); p.first != p.second; ++p.first)
    {
        assert(nodeType(*p.first) == NODE_Clique);
        assert(countMap.count(*p.first) != 0);
        assert(countMap[*p.first] <= count);

        if(count != countMap[*p.first])
            toRemove.push_back(*p.first);
    }

    for(std::list<NodeDescriptor>::const_iterator it = toRemove.begin(); it != toRemove.end(); ++it)
        cleanSubtree(*it);
}

void DecompositionDAG::checkSeparatorNode(NodeDescriptor node, boost::unordered_map<NodeDescriptor, std::size_t> & countMap)
{
    typedef boost::graph_traits<Structure>::adjacency_iterator adjIt;

    const SeparatorNodeData & data = *separatorNodeData(node);

    std::size_t count = 0;
    for(std::pair<adjIt, adjIt> p = boost::adjacent_vertices(node, dag_); p.first != p.second; ++p.first)
    {
        assert(nodeType(*p.first) == NODE_Subgraph && countMap.count(*p.first) != 0);
        count += countMap[*p.first];
    }

    if(count < data.separator.size())
        cleanSubtree(node);
    else
        countMap.insert(std::make_pair(node, count - data.separator.size()));
}

void DecompositionDAG::checkCliqueNode(NodeDescriptor node, boost::unordered_map<NodeDescriptor, std::size_t> & countMap)
{
    typedef boost::graph_traits<Structure>::adjacency_iterator adjIt;

    // get the clique size
    std::size_t cliqueSize = cliqueMap_.find(node)->second.size();

    std::size_t count = 0;
    for(std::pair<adjIt, adjIt> p = boost::adjacent_vertices(node, dag_); p.first != p.second; ++p.first)
    {
        assert(nodeType(*p.first) == NODE_Separator);
        assert(countMap.count(*p.first) != 0);
        count += countMap[*p.first];
    }

    if(count < 1)
        cleanSubtree(node);
    else
        countMap.insert(std::make_pair(node, count + cliqueSize));
}

void DecompositionDAG::cleanSubtree(NodeDescriptor node)
{
    typedef boost::graph_traits<Structure>::vertex_iterator vit;
    typedef boost::graph_traits<Structure>::adjacency_iterator adjIt;

    // create a degree map for the nodes
    boost::unordered_map<NodeDescriptor, std::size_t> inDegreeMap;

    // set the degree of the node to zero
    inDegreeMap[node] = 1;
    std::stack<NodeDescriptor> todo;
    todo.push(node);

    std::list<NodeDescriptor> toRemove;

    std::size_t processed = 0;

    // process a stack
    while(!todo.empty())
    {
        NodeDescriptor curNode = todo.top();
        todo.pop();

        ++processed;

        // have we already seen this node?
        boost::unordered_map<NodeDescriptor, std::size_t>::iterator it = inDegreeMap.find(curNode);
        if(it == inDegreeMap.end())
            it = inDegreeMap.insert(std::make_pair(curNode, boost::in_degree(curNode, dag_))).first;

        // check the counter
        std::size_t & curD = it->second;

        // increase the counter
        --curD;

        // last time in the stack, all parents seen so process
        if(curD == 0)
        {
            // schedule this node for removal
            toRemove.push_back(curNode);

            // add the children to the list
            for(std::pair<adjIt, adjIt> p = boost::adjacent_vertices(curNode, dag_); p.first != p.second; ++p.first)
                todo.push(*p.first);
        }
    }

    // okay, all nodes in to remove can be removed
    for(std::list<NodeDescriptor>::const_iterator it = toRemove.begin(); it != toRemove.end(); ++it)
    {
        boost::clear_vertex(*it, dag_);
        boost::remove_vertex(*it, dag_);
    }
}

void DecompositionDAG::cleanupParallelEdges()
{
    typedef boost::graph_traits<Structure>::vertex_iterator vit;
    typedef std::pair<NodeDescriptor, NodeDescriptor>SubgraphCliquePair;
    typedef boost::graph_traits<Structure>::adjacency_iterator adjIt;

    std::list<SubgraphCliquePair > toConsider;

    // loop over all subgraph nodes
    for(std::pair<vit,vit> p = boost::vertices(dag_); p.first != p.second; ++p.first)
    {
        // only subgraph nodes
        NodeDescriptor node = *p.first;
        if(nodeType(node) != NODE_Subgraph)
            continue;

        const SubgraphNodeData & data = *subgraphNodeData(node);

        // consider all attached cliques
        for(std::pair<adjIt, adjIt> pc = boost::adjacent_vertices(node, dag_); pc.first != pc.second; ++pc.first)
        {
            // get the clique
            const VertexSet & clique = cliqueMap_.find(*pc.first)->second;

            if(clique.size() > data.activeVertices.size())
                toConsider.push_back(std::make_pair(node, *pc.first));
        }
    }

    std::list<NodeDescriptor> toRemove;

    // and now process them
    for(std::list<SubgraphCliquePair >::const_iterator it =  toConsider.begin(); it != toConsider.end(); ++it)
        if(hasSmallerTreewidthParallelEdge(it->first, it->second))
            toRemove.push_back(it->second);

    // remove everything unneccairy
    for(std::list<NodeDescriptor>::const_iterator it = toRemove.begin(); it != toRemove.end(); ++it)
        cleanSubtree(*it);
}

bool DecompositionDAG::hasSmallerTreewidthParallelEdge(NodeDescriptor subgraphNode, NodeDescriptor cliqueNode) const
{
    typedef boost::graph_traits<Structure>::adjacency_iterator adjIt;

    // all the necessary information
    const VertexSet & clique = cliqueMap_.find(cliqueNode)->second;

    // get all the separator nodes attached to the clique
    boost::unordered_set<NodeDescriptor> attachedSeparators;
    for(std::pair<adjIt, adjIt> p = boost::adjacent_vertices(cliqueNode, dag_); p.first != p.second; ++p.first)
        attachedSeparators.insert(*p.first);

    // create a stack to process
    std::stack<NodeDescriptor> todo;
    boost::unordered_set<NodeDescriptor> processed;
    todo.push(subgraphNode);

    while(!todo.empty())
    {
        // get the node
        NodeDescriptor cur = todo.top();
        todo.pop();

        // first time this node?
        if(!processed.insert(cur).second)
            continue;

        assert(nodeType(cur) == NODE_Subgraph);

        // loop over the clique nodes adjacent to this subgraph
        for(std::pair<adjIt, adjIt> pc = boost::adjacent_vertices(cur, dag_); pc.first != pc.second; ++pc.first)
        {
            // get the clique
            NodeDescriptor nClique = *pc.first;
            const VertexSet & dClique = cliqueMap_.find(nClique)->second;

            // the clique should be a strict subset
            if(dClique.size() >= clique.size() || !isSubset(dClique, clique))
                continue;

            // first loop over all attached to this clique
            for(std::pair<adjIt, adjIt> ps = boost::adjacent_vertices(nClique, dag_); ps.first != ps.second; ++ps.first)
            {
                NodeDescriptor nSeparator = *ps.first;

                // remove if it was attached
                attachedSeparators.erase(nSeparator);

                // maybe we have finished
                if(attachedSeparators.empty())
                    return true;

                // consider all the attached subgraphs
                for(std::pair<adjIt, adjIt> pg = boost::adjacent_vertices(nSeparator, dag_); pg.first != pg.second; ++pg.first)
                    todo.push(*pg.first);
            }
        }
    }

    return false;

}


} // namespace treeDAG
