/*******************************************************************
* Copyright (c) 2010-2013 MiGraNT - DTAI.cs.kuleuven.be
* License details in the root directory of this distribution in
* the LICENSE file
********************************************************************/

#ifndef TREEDAG_UTIL_KPERMUTATEITERATOR_HPP
#define TREEDAG_UTIL_KPERMUTATEITERATOR_HPP

#include <vector>
#include <utility>
#include <stdexcept>
#include <cassert>

#include <boost/iterator/iterator_facade.hpp>

namespace treeDAG {
namespace util {

template <typename Iterator> class KOrderedPermutateIterator
        : public boost::iterator_facade<
                KOrderedPermutateIterator<Iterator>,
                std::vector<typename std::iterator_traits<Iterator>::value_type>,
                boost::forward_traversal_tag,
                std::vector<typename std::iterator_traits<Iterator>::value_type>
            >
{
public:
    KOrderedPermutateIterator(Iterator first, Iterator last, unsigned int k)
        : end_(last),
          k_(k)
    {
        if(k > std::distance(first, last))
            throw std::out_of_range("KPermutateIterator: The range is smaller than the supplied k");

        permutation_.assign(k, first);
        initialiseFromPositionOnwards(0);
    }

    KOrderedPermutateIterator(const std::pair<Iterator, Iterator> & iterators, unsigned int k)
        : end_(iterators.second),
          k_(k)
    {
        if(k > std::distance(iterators.first, iterators.second))
            throw std::out_of_range("KPermutateIterator: The range is smaller than the supplied k");

        if(k <= 0)
            return;

        permutation_.assign(k, iterators.first);
        initialiseFromPositionOnwards(0);
    }

    KOrderedPermutateIterator()
    {
    }

    const std::vector<Iterator> & currentPermutationIterators() const
    {
        return permutation_;
    }

private:
    friend class boost::iterator_core_access;
    bool increment()
    {
        // find the first that can be incremented (from the back on)
        unsigned int dist = 1 ;
        for(; dist <= k_; ++dist)
        {
            if(std::distance(permutation_[k_-dist], end_) > dist)
                break;
        }

        // did we find a good position?
        if(dist > k_)
        {
            permutation_.back() = end_;
            return false;
        }

        unsigned int pos = k_ - dist;

        // update all the next positions
        ++permutation_[pos];
        bool possible = initialiseFromPositionOnwards(pos);

        assert(possible);
        return true;
    }

    bool initialiseFromPositionOnwards(unsigned int pos)
    {
        Iterator cur = permutation_[pos];

        while(++pos < k_ && ++cur != end_)
            permutation_[pos] = (cur);

        return (pos == k_);
    }

    bool equal(const KOrderedPermutateIterator<Iterator> & other) const
    {
        // they are equal
        if(permutation_ == other.permutation_ && end_ == other.end_)
            return true;

        // one of us is at end?
        return atEnd() && other.atEnd();
    }

    bool atEnd() const
    {
        if(permutation_.empty())
            return true;

        return permutation_.back() == end_;
    }

    std::vector<typename std::iterator_traits<Iterator>::value_type> dereference() const
    {
        std::vector<typename std::iterator_traits<Iterator>::value_type> vct(permutation_.size());
        for(std::size_t i = 0; i < k_; ++i)
            vct[i] = *permutation_[i];

        return vct;
    }

private:
    std::vector<Iterator> permutation_;
    Iterator end_;
    unsigned int k_;
};

template <typename Iterator>
std::pair<KOrderedPermutateIterator<Iterator>, KOrderedPermutateIterator<Iterator> >
make_k_ordered_permutate_range(Iterator first, Iterator last, unsigned int k)
{
    if(k == 0)
        return std::make_pair(KOrderedPermutateIterator<Iterator>(), KOrderedPermutateIterator<Iterator>());
    else
        return std::make_pair(KOrderedPermutateIterator<Iterator>(first, last, k), KOrderedPermutateIterator<Iterator>());
}

template <typename Iterator>
std::pair<KOrderedPermutateIterator<Iterator>, KOrderedPermutateIterator<Iterator> >
make_k_ordered_permutate_range(const std::pair<Iterator, Iterator> & iterators, unsigned int k)
{
    return std::make_pair(KOrderedPermutateIterator<Iterator>(iterators, k), KOrderedPermutateIterator<Iterator>());
}

} // util namespace
} // treeDAG namespace

#endif // TREEDAG_UTIL_KPERMUTATEITERATOR_HPP
