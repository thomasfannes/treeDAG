#include "combinationIterator.hpp"

namespace treeDAG {
namespace util {

bool CombinationMaskProcessor::atEnd(const std::vector<bool> & mask) const
{
    return mask.back();
}

void CombinationMaskProcessor::increment(std::vector<bool> & mask)
{
    const std::size_t sz = mask.size();

    // find the first false value
    std::size_t flipPos = 0;
    while(mask[flipPos] && flipPos < sz)
        ++flipPos;

    // can we flip it to true?
    if(flipPos < sz)
        mask[flipPos] = true;

    // set the rest to false
    std::fill(mask.begin(), mask.begin() + flipPos, false);
}

} // util namespace
} // treeDAG namespace
