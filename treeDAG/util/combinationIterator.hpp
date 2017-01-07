#ifndef TREEDAG_UTIL_COMBINATIONITERATOR_HPP
#define TREEDAG_UTIL_COMBINATIONITERATOR_HPP

#include <vector>
#include <boost/iterator/iterator_facade.hpp>

namespace treeDAG {
namespace util {

template <typename Iterator> struct CombinationIterator;

template <typename Iterator>
struct CombinationValueIterator
        : public boost::iterator_facade
        <
            CombinationValueIterator<Iterator>,
            typename std::iterator_traits<Iterator>::value_type,
            boost::forward_traversal_tag,
            typename std::iterator_traits<Iterator>::reference
        >
{
    CombinationValueIterator(Iterator first, const std::vector<bool> * mask);
    CombinationValueIterator();

private:
    friend class boost::iterator_core_access;
    void increment();
    bool equal(const CombinationValueIterator<Iterator> & other) const;
    typename std::iterator_traits<Iterator>::reference dereference() const;

    bool atEnd() const;
    void gotoFirstValid();

private:
    Iterator cur_;
    std::size_t pos_;
    const std::vector<bool> * mask_;
};


template <typename Iterator>
class CombinationValueProxy
{
    CombinationValueProxy(const CombinationIterator<Iterator> * base);

public:

    typedef CombinationValueIterator<Iterator> iterator;

    std::pair<iterator, iterator> range() const;
    iterator begin() const;
    iterator end() const;

private:
    friend class CombinationIterator<Iterator>;

    const CombinationIterator<Iterator> * base_;
};

template <typename Iterator>
class CombinationIterator : public boost::iterator_facade
        <
            CombinationIterator<Iterator>,
            CombinationValueProxy<Iterator>,
            boost::forward_traversal_tag,
            const CombinationValueProxy<Iterator> &
        >
{
public:
    CombinationIterator();
    CombinationIterator(const std::pair<Iterator, Iterator> & range);
    CombinationIterator(Iterator first, Iterator last);
    CombinationIterator(const CombinationIterator & other);

    CombinationIterator & operator=(const CombinationIterator & other);

private:
    friend class CombinationValueProxy<Iterator>;
    friend class boost::iterator_core_access;

    void increment();
    bool equal(const CombinationIterator<Iterator> & other) const;
    const CombinationValueProxy<Iterator> & dereference() const;

    bool atEnd() const;

    Iterator first_;
    std::vector<bool> mask_;
    CombinationValueProxy<Iterator> proxy_;
};

template <typename Iterator>
std::pair<CombinationIterator<Iterator>, CombinationIterator<Iterator> > make_combination_iterators(Iterator first, Iterator last)
{
    return std::make_pair(CombinationIterator<Iterator>(first, last), CombinationIterator<Iterator>());
}

template <typename Iterator>
std::pair<CombinationIterator<Iterator>, CombinationIterator<Iterator> > make_combination_iterators(const std::pair<Iterator, Iterator> & range)
{
    return std::make_pair(CombinationIterator<Iterator>(range), CombinationIterator<Iterator>());
}

} // namespace util
} // namespace treeDAG



#include "combinationIterator.hxx"

#endif // TREEDAG_UTIL_COMBINATIONITERATOR_HPP
