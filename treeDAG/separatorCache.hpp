#ifndef TREEDAG_SEPARATORCACHE_HPP
#define TREEDAG_SEPARATORCACHE_HPP

#include "separation.hpp"

namespace treeDAG {

class SeparatorCache : public SeparatorConfig
{
    typedef boost::unordered_set<Separation, SeparationHash, SeparationEqual> SeparatorMap;

public:
    typedef typename SeparatorMap::const_iterator SeparatorIterator;
    typedef SeparatorConfig::Graph Graph;

    SeparatorCache();
    SeparatorCache(std::size_t k, const Graph * graph);

    void initialize();

    const Separation * findSeparator(const VertexSet & separator) const;
    std::pair<SeparatorIterator, SeparatorIterator> separators() const;

private:
    void processPossibleSeparator(const std::vector<VertexIndexType> &possibleSeparator);

    const Graph * graph_;
    SeparatorMap map_;
    std::size_t k_;
};

} // namespace treeDAG


#endif // TREEDAG_SEPARATORCACHE_HPP
