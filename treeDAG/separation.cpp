#include "separation.hpp"

namespace treeDAG {
namespace  {

typedef SeparatorConfig::VertexSet VertexSet;

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

} //  namespace





void Separation::limitToMaximalComponents()
{
    // we create a map from old index -> new index
    std::vector<std::size_t> translationMap(components.size(), UnassignedVertex());
    std::size_t newComp = 0;

    for(std::size_t oldComp = 0; oldComp < components.size(); ++oldComp)
    {
        // is this a good subset
        if(isSubset(separator, components[oldComp]))
            // then store the translation
            translationMap[oldComp] = newComp++;
    }

    // and now process the component map
    for(std::vector<VertexIndexType>::iterator it = componentMap.begin(); it != componentMap.end(); ++it)
        // no separator
        if(*it != SeparatorVertex())
        {
            // should we set it to unassigned?
            if(translationMap[*it] == UnassignedVertex())
                *it = UnassignedVertex();
            // should we change the component according to the translation map
            else
                *it = translationMap[*it];
        }

    // finally move all the unwanted components to the end
    std::size_t tgtComp = 0;
    for(std::size_t curComp = 0; curComp < components.size(); ++curComp)
    {
        // if it one we should not be interested in, just skip it
        if(translationMap[curComp] == UnassignedVertex())
            continue;

        assert(translationMap[curComp] == tgtComp);
        std::swap(components[curComp], components[tgtComp]);
        ++tgtComp;
    }

    // and remove the unwanted components
    components.erase(components.begin() + tgtComp, components.end());
}



bool operator==(const Separation & lhs, const Separation & rhs)
{
    return lhs.separator == rhs.separator;
}

std::size_t hash_value(const Separation & separation)
{
    return SeparationHash()(separation);
}


bool SeparationEqual::operator()(const Separation & lhs, const Separation & rhs) const
{
    return operator ()(lhs.separator, rhs.separator);
}

bool SeparationEqual::operator()(const VertexSet & lhs, const Separation & rhs) const
{
    return operator ()(lhs, rhs.separator);
}

bool SeparationEqual::operator()(const Separation & lhs, const VertexSet & rhs) const
{
    return operator ()(lhs.separator, rhs);
}

bool SeparationEqual::operator()(const VertexSet & lhs, const VertexSet & rhs) const
{
    return lhs == rhs;
}

std::size_t SeparationHash::operator()(const Separation & separation) const
{
    return operator ()(separation.separator);
}

std::size_t SeparationHash::operator()(const VertexSet & separation) const
{
    return boost::hash_value(separation);
}

} // treeDAG namespace
