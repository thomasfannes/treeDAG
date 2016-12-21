#ifndef TREEDAG_SEPARATORCACHE_HPP
#define TREEDAG_SEPARATORCACHE_HPP

#include "separation.hpp"
#include "util/tvsArray.hpp"

namespace treeDAG {

template <int SIZE, typename VertexIndexType = SeparatorConfig::VertexIndexType>
class SeparatorCache
{
    typedef std::vector<VertexIndexType> KeyType;
    typedef std::vector<VertexIndexType> VertexVector;

    struct KeyHasher
    {
        std::size_t operator()(const KeyType & keyType) const;
        template <typename VertexType>
        std::size_t operator()(const std::vector<VertexType> & keyType) const;
    };

    struct KeyEquality
    {
        template <typename VertexType1, typename VertexType2>
        bool operator()(const std::vector<VertexType1> & lhs, const std::vector<VertexType2> & rhs) const;
    };



public:
    typedef SeparatorConfig::Graph Graph;
    typedef std::list<VertexVector> ComponentList;

    SeparatorCache();
    SeparatorCache(std::size_t k, const Graph * graph);

    void initialize();

    template <typename VertexType>
    const ComponentList * findSeparator(const std::vector<VertexType> & separator) const;

private:
    void processPossibleSeparator(const std::vector<VertexIndexType> & possibleSeparator);
    bool hasIntersection(const VertexVector & lhs, const VertexVector & rhs) const;

    typedef boost::unordered_map<KeyType, ComponentList, KeyHasher> SeparatorMap;

    const Graph * graph_;
    SeparatorMap map_;
    std::size_t k_;
};

} // namespace treeDAG

#include "separatorCache.hxx"

#endif // TREEDAG_SEPARATORCACHE_HPP
