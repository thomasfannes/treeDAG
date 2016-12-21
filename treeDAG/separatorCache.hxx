#ifndef TREEDAG_SEPARATORCACHE_HXX
#define TREEDAG_SEPARATORCACHE_HXX

#include "separatorCache.hpp"
#include "util/kOrderedPermutateIterator.hpp"
#include "separator.hpp"

namespace treeDAG {

#define TDEF template <int SIZE,  typename VertexIndexType>
#define CDEF SeparatorCache<SIZE, VertexIndexType>

TDEF
std::size_t CDEF::KeyHasher::operator()(const KeyType & keyType) const
{
    std::size_t seed = boost::hash_value( static_cast<VertexIndexType>(keyType.size()) );

    for(std::size_t i = 0; i < keyType.size(); ++i)
        boost::hash_combine(seed, static_cast<VertexIndexType>(keyType[i]));

    return seed;
}

TDEF
template <typename VertexType>
std::size_t CDEF::KeyHasher::operator()(const std::vector<VertexType> & keyType) const
{
    std::size_t seed = boost::hash_value( static_cast<VertexIndexType>(keyType.size()) );

    for(std::size_t i = 0; i < keyType.size(); ++i)
        boost::hash_combine(seed, static_cast<VertexIndexType>(keyType[i]));

    return seed;
}



TDEF
template <typename VertexType1, typename VertexType2>
bool CDEF::KeyEquality::operator()(const std::vector<VertexType1> & lhs, const std::vector<VertexType2> & rhs) const
{
    if(lhs.size() != rhs.size())
        return false;

    for(std::size_t i = 0; i < lhs.size(); ++i)
        if(lhs[i] != rhs[i])
            return false;

    return true;

}


TDEF
CDEF::SeparatorCache()
    : graph_(0),
      k_(0)
{
}

TDEF
CDEF::SeparatorCache(std::size_t k, const Graph * graph)
    : graph_(graph),
      k_(k)
{
}

TDEF
void CDEF::initialize()
{
    // initialize for all possible permutations of graph vertices
    typedef boost::graph_traits<Graph>::vertex_iterator Vit;
    typedef util::KOrderedPermutateIterator<typename std::vector<VertexIndexType>::iterator> PermIter;

    // map the graph vertices
    std::vector<VertexIndexType> graphVertices;
    std::pair<Vit, Vit> p = boost::vertices(*graph_);
    graphVertices.assign(p.first, p.second);

    // for all possible k values
    for(std::size_t curK = 1; curK <= k_; ++curK)
        // loop over all permutations of (graphSize choose curK)
        for(std::pair<PermIter, PermIter> p = util::make_k_ordered_permutate_range(graphVertices.begin(), graphVertices.end(), curK); p.first != p.second; ++p.first)
            processPossibleSeparator(*p.first);
}


TDEF
void
CDEF::processPossibleSeparator(const std::vector<VertexIndexType> &possibleSeparator)
{
    // we need a separator
    Separator<VertexIndexType> separate(graph_);

    // find all the components
    const std::list<VertexVector> & components = separate.findMaximalComponents(possibleSeparator.begin(), possibleSeparator.end());

    // only one component, then no minimal separator
    if(components.size() <= 1)
        return;

    // and add the different components
    map_.insert(std::make_pair(possibleSeparator, components));
}

TDEF
template <typename VertexType>
const typename CDEF::ComponentList *
CDEF::findSeparator(const std::vector<VertexType> & separator) const
{
    typename SeparatorMap::const_iterator it = map_.find(separator, KeyHasher(), KeyEquality());
    if(it == map_.end())
        return 0;

    return &it->second;
}


#undef TDEF
#undef CDEF

} // namespace treeDAG

#endif // TREEDAG_SEPARATORCACHE_HXX
