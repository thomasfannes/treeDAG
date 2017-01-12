
#include "decomposer.hpp"
#include "util/nChooseKIterator.hpp"
#include "util/combinationIterator.hpp"


namespace treeDAG {

Decomposer::Decomposer(const Graph * graph, std::size_t k)
    : cache_(k, graph),
      k_(k),
      graph_(graph)
{
}

Decomposer::Decomposer()
    : k_(0),
      graph_(0)
{
}

void  Decomposer::initialize()
{
    cache_.initialize();
}


void Decomposer::writeDot(std::ostream & stream) const
{
    dag_.write_dot(stream);
}

void Decomposer::processRoot()
{
    typedef util::NChooseKIterator<VertexSet::const_iterator> it;

    // create the root graph
    SubgraphNodeData data;
    const std::size_t graphSize = boost::num_vertices(*graph_);
    VertexSet::const_iterator sepIt = roots_.begin();

    // loop over all vertices
    for(std::size_t curV = 0; curV < graphSize; ++curV)
    {
        // distribute between other and active vertices
        if(sepIt == roots_.end() || *sepIt > curV)
            data.otherVertices.push_back(curV);
        // it is an active vertex
        else
            data.activeVertices.push_back(*sepIt++);
    }

    DecompositionDAG::NodeDescriptor node = dag_.addSubgraph(data);

    // storage for the already added
    boost::unordered_set<UsedSeparatorNodeSet> addedCliqueNodes;

    // loop over all combinations
    const std::size_t rootSize = roots_.size();
    const std::size_t maxToAdd = k_ + 1 - rootSize;
    const VertexSet & otherVertices = data.otherVertices;

    for(std::size_t extraCount = 0; extraCount <= maxToAdd; ++extraCount)
    {
        for(std::pair<it, it> p = util::make_n_choose_k_iterators(otherVertices.begin(), otherVertices.end(), extraCount); p.first != p.second; ++p.first)
        {
            const VertexSet & extraVertices = *p.first;

            VertexSet newClique(rootSize + extraCount);
            std::merge(data.activeVertices.begin(), data.activeVertices.end(), extraVertices.begin(), extraVertices.end(), newClique.begin());

            tryClique(node, VertexSet(), newClique, addedCliqueNodes);
        }
    }

}

void Decomposer::process(DecompositionDAG::NodeDescriptor node)
{
    boost::unordered_set<UsedSeparatorNodeSet> addedCliqueNodes;

    // get the data
    const SubgraphNodeData & data = *dag_.subgraphNodeData(node);

    // how many to add?
    std::size_t toAdd = k_ - data.activeVertices.size() + 1;
    assert(toAdd > 0);

    // loop over all possible nodes to add
    typedef util::NChooseKIterator<VertexSet::const_iterator> it;


    // we will try clique sizes from activecount+1 -> k + 1
    for(std::size_t cur = 1; cur <= toAdd; ++cur)
    {
        // are there still enough other vertices left to choose from?
        if(data.otherVertices.size() <= cur)
            break;

        for(std::pair<it, it> p = util::make_n_choose_k_iterators(data.otherVertices.begin(), data.otherVertices.end(), cur); p.first != p.second; ++p.first)
            tryClique(node, data.activeVertices, *p.first, addedCliqueNodes);
    }
}


void Decomposer::tryClique(DecompositionDAG::NodeDescriptor subgraphNode, const VertexSet & oldVertices, const VertexSet & newVertices, boost::unordered_set<UsedSeparatorNodeSet> & cache)
{
    std::size_t curK = oldVertices.size() + newVertices.size() - 1;

    typedef util::CombinationIterator<VertexSet::const_iterator> CombIt;

    std::vector<VertexIndexType> clique;
    std::merge(oldVertices.begin(), oldVertices.end(), newVertices.begin(), newVertices.end(), std::back_inserter(clique));

    UsedSeparatorNodeSet usedSeparators;

    // first loop over all combinations of the new vertices
    for(CombIt newIt = CombIt(newVertices.begin(), newVertices.end()); newIt != CombIt(); ++newIt)
    {
        // extract the new vertices
        VertexSet curNew(newIt->begin(), newIt->end());

        // make sure that there is at least one new vertex (otherwise we should not check for separation)
        if(curNew.empty())
            continue;

        // now loop over all combinations of the old vertices
        for(CombIt oldIt = CombIt(oldVertices.begin(), oldVertices.end()); oldIt != CombIt(); ++oldIt)
        {
            // extract the old vertices
            VertexSet curOld(oldIt->begin(), oldIt->end());

            // did we select all vertices
            if(curOld.size() + curNew.size() > curK)
                continue;

            // create the possible separator
            VertexSet possibleSeparator;
            std::merge(curNew.begin(), curNew.end(), curOld.begin(), curOld.end(), std::back_inserter(possibleSeparator));

            // and now try this separator
            trySeparator(possibleSeparator, clique, usedSeparators);
        }
    }


    // nothing found
    if(usedSeparators.empty())
        return;

    // did we already try this combination for this subgraph?
    if(!cache.insert(usedSeparators).second)
        return;

    // okay, a new combination so add it
    dag_.addClique(subgraphNode, clique, usedSeparators.begin(), usedSeparators.end());
}


void Decomposer::trySeparator(const VertexSet & possibleSeparator, const VertexSet & clique, UsedSeparatorNodeSet & usedSeparators)
{
    // do we have this separator
    const Separation * separation = cache_.findSeparator(possibleSeparator);
    if(separation == 0)
        return;

    // extract the non-separator vertices
    VertexSet nonSeparatorVertices;
    std::set_difference(clique.begin(), clique.end(), possibleSeparator.begin(), possibleSeparator.end(), std::back_inserter(nonSeparatorVertices));
    assert(!nonSeparatorVertices.empty());

    // okay, now find the single component have the non-SeparatorVertices
    std::set<VertexIndexType> inactiveComponents;
    for(VertexSet::const_iterator it = nonSeparatorVertices.begin(); it != nonSeparatorVertices.end(); ++it)
        if(separation->componentMap[*it] != separation->UnassignedVertex())
            inactiveComponents.insert(separation->componentMap[*it]);


    // and add this separator
    usedSeparators.insert(addSeparatorNode(*separation, VertexSet(inactiveComponents.begin(), inactiveComponents.end())));
}


DecompositionDAG::NodeDescriptor Decomposer::addSeparatorNode(const Separation & separation, const VertexSet & inactiveIndices)
{
    // create the separator node data
    SeparatorNodeData sepData;
    sepData.separator = separation.separator;
    sepData.inactiveComponents = inactiveIndices;

    // have we already processed this?
    DecompositionDAG::NodeDescriptor sepNode = dag_.findSeparatorNode(sepData);
    if(sepNode != DecompositionDAG::InvalidNode())
        return sepNode;

    // create or find the subgraph nodes
    std::list<DecompositionDAG::NodeDescriptor> subgraphNodes;
    std::list<SubgraphNodeData> subgraphs;

    VertexSet::const_iterator inactiveIt = inactiveIndices.begin();

    // loop over all components
    for(std::size_t i = 0; i < separation.components.size(); ++i)
    {
        assert(inactiveIt == inactiveIndices.end() || *inactiveIt <= i);

        // is this an inactive index?
        if(inactiveIt != inactiveIndices.end() && *inactiveIt == i)
        {
            ++inactiveIt;
            continue;
        }

        // no so find or create the subgraph
        const SubgraphNodeData & nodeData = createSubgraphNodeData(separation.separator, separation.components[i]);

        // add to the list of subgraphs
        subgraphs.push_back(nodeData);

        // and add the node id also
        DecompositionDAG::NodeDescriptor nd = dag_.findSubgraphNode(nodeData);
        if(nd == dag_.InvalidNode())
        {
            // no, so create it
            nd = dag_.addSubgraph(nodeData);

            // and add it to be processed
            todo_.push(nd);
        }

        subgraphNodes.push_back(nd);
    }

    // and add separator
    return dag_.addSeparator(sepData, subgraphs.begin(), subgraphs.end());
}

SubgraphNodeData Decomposer::createSubgraphNodeData(const VertexSet & separator, const VertexSet & component)
{
    SubgraphNodeData data;

    data.activeVertices = separator;
    std::set_difference(component.begin(), component.end(), separator.begin(), separator.end(), std::back_inserter(data.otherVertices));

    return data;
}

} // namespace treeDAG

