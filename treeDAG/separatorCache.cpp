#include "separatorCache.hpp"
#include "util/kOrderedPermutateIterator.hpp"
#include "separator.hpp"

namespace treeDAG {


SeparatorCache::SeparatorCache()
    : graph_(0),
      k_(0)
{
}


SeparatorCache::SeparatorCache(std::size_t k, const Graph * graph)
    : graph_(graph),
      k_(k)
{
}


void SeparatorCache::initialize()
{    
    // initialize for all possible permutations of graph vertices
    typedef boost::graph_traits<Graph>::vertex_iterator Vit;
    typedef util::KOrderedPermutateIterator<typename std::vector<VertexIndexType>::iterator> PermIter;

    // store all the graph vertices
    VertexSet graphVertices;
    std::pair<Vit, Vit> p = boost::vertices(*graph_);
    graphVertices.assign(p.first, p.second);

    // loop over all permutations of (graphSize choose curK)
    for(std::size_t curK = 1; curK <= k_; ++curK)
        for(std::pair<PermIter, PermIter> p = util::make_k_ordered_permutate_range(graphVertices.begin(), graphVertices.end(), curK); p.first != p.second; ++p.first)
            processPossibleSeparator(*p.first);
}


void SeparatorCache::processPossibleSeparator(const std::vector<VertexIndexType> &possibleSeparator)
{
    // we need a separator
    Separator separate(graph_);

    // find all the maximal components
    Separation separation = separate(possibleSeparator.begin(), possibleSeparator.end());
    separation.limitToMaximalComponents();

    // only one component, then no minimal separator
    if(separation.components.size() <= 1)
        return;

    // now create a new separator and add it
    map_.insert(separation);
}


const SeparatorCache::ComponentSet *
SeparatorCache::findSeparator(const VertexSet & separator) const
{
    typename SeparatorMap::const_iterator it = map_.find(separator, SeparationHash(), SeparationEqual());
    if(it == map_.end())
        return 0;

    return &it->components;
}

std::pair<SeparatorCache::SeparatorIterator, SeparatorCache::SeparatorIterator>
SeparatorCache::separators() const
{
    return std::make_pair(map_.begin(), map_.end());
}



} // namespace treeDAG

