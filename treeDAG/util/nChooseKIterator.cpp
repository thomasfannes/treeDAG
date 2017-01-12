
#include "nChooseKIterator.hpp"
#include <algorithm>

namespace treeDAG {
namespace util {

bool NChooseKProcessor::atEnd(const std::vector<bool> & mask) const
{
    return mask.back();
}

void NChooseKProcessor::increment(std::vector<bool> & mask)
{
    mask.back() = !std::next_permutation(mask.begin(), mask.end() - 1);
}

} // util namespace
} // treeDAG namespace

