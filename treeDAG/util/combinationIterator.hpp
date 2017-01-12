#ifndef TREEDAG_UTIL_COMBINATIONITERATOR_HPP
#define TREEDAG_UTIL_COMBINATIONITERATOR_HPP

#include "maskedRangeProxy.hpp"

namespace treeDAG {
namespace util {

struct CombinationMaskProcessor
{
    bool atEnd(const std::vector<bool> & mask) const;
    void increment(std::vector<bool> & mask);
};

template <typename Iterator>
struct CombinationIterator
        : public MaskedRangeIterator<CombinationMaskProcessor, Iterator>
{
    typedef MaskedRangeIterator<CombinationMaskProcessor, Iterator> Base;

public:
    CombinationIterator();
    CombinationIterator(Iterator first, Iterator last);
};

template <typename Iterator>
std::pair<CombinationIterator<Iterator>, CombinationIterator<Iterator> > make_combination_iterators(Iterator first, Iterator last)
{
    return std::make_pair(CombinationIterator<Iterator>(first, last), CombinationIterator<Iterator>());
}

template <typename Iterator>
std::pair<CombinationIterator<Iterator>, CombinationIterator<Iterator> > make_combination_iterators(const std::pair<Iterator, Iterator> & range)
{
    return std::make_pair(CombinationIterator<Iterator>(range.first, range.second), CombinationIterator<Iterator>());
}

} // namespace util
} // namespace treeDAG



#include "combinationIterator.hxx"

#endif // TREEDAG_UTIL_COMBINATIONITERATOR_HPP
