#ifndef TREEDAG_UTIL_NCHOOSEKITERATOR_HPP
#define TREEDAG_UTIL_NCHOOSEKITERATOR_HPP

#include "maskedRangeIterator.hpp"


namespace treeDAG {
namespace util {

struct NChooseKProcessor
{
    bool atEnd(const std::vector<bool> & mask) const;
    void increment(std::vector<bool> & mask);
};

template <typename Iterator>
class NChooseKIterator : public MaskedRangeIterator<NChooseKProcessor, Iterator>
{
    typedef MaskedRangeIterator<NChooseKProcessor, Iterator> Base;

public:
    NChooseKIterator();
    NChooseKIterator(Iterator first, Iterator last, std::size_t k);
    NChooseKIterator(Iterator first, std::size_t n, std::size_t k);
};


template <typename Iterator>
std::pair<NChooseKIterator<Iterator>, NChooseKIterator<Iterator> > make_n_choose_k_iterators(Iterator first, Iterator last, std::size_t k)
{
    return std::make_pair(NChooseKIterator<Iterator>(first, last, k), NChooseKIterator<Iterator>());
}

template <typename Iterator>
std::pair<NChooseKIterator<Iterator>, NChooseKIterator<Iterator> > make_n_choose_k_iterators(Iterator first, std::size_t n, std::size_t k)
{
    return std::make_pair(NChooseKIterator<Iterator>(first, n, k), NChooseKIterator<Iterator>());
}

template <typename Iterator>
std::pair<NChooseKIterator<Iterator>, NChooseKIterator<Iterator> > make_n_choose_k_iterators(const std::pair<Iterator, Iterator> & range, std::size_t k)
{
    return std::make_pair(NChooseKIterator<Iterator>(range.first, range.second, k), NChooseKIterator<Iterator>());
}


} // namespace util
} // namespace treeDAG

#include "nChooseKIterator.hxx"

#endif // TREEDAG_UTIL_NCHOOSEKITERATOR_HPP
