#ifndef TREEDAG_UTIL_MASKEDRANGEPROXY_HPP
#define TREEDAG_UTIL_MASKEDRANGEPROXY_HPP

#include "maskedRangeIterator.hpp"

namespace treeDAG {
namespace util {

template <typename Iterator> class MaskedRangeIteratorBase;

template <
        typename Iterator,
        typename Container = std::vector<typename std::iterator_traits<Iterator>::value_type>
        >
class MaskedRangeProxy
{
public:
    struct iterator : public boost::iterator_facade
            <
            iterator,
            typename std::iterator_traits<Iterator>::value_type,
            boost::forward_traversal_tag,
            typename std::iterator_traits<Iterator>::reference
            >
    {
        iterator(Iterator first, const std::vector<bool> * mask);
        iterator();

    private:
        friend class boost::iterator_core_access;

        void increment();
        bool equal(const iterator & other) const;
        typename std::iterator_traits<Iterator>::reference dereference() const;

        bool atEnd() const;
        void gotoFirstValid();

    private:
        Iterator cur_;
        std::size_t pos_;
        const std::vector<bool> * mask_;
    };

private:
    MaskedRangeProxy(const MaskedRangeIteratorBase<Iterator> & base);
    MaskedRangeProxy(const MaskedRangeProxy<Iterator, Container> & rhs);
    MaskedRangeProxy<Iterator, Container> & operator=(const MaskedRangeProxy<Iterator, Container> & rhs);

public:
    std::pair<iterator, iterator> range() const;
    iterator begin() const;
    iterator end() const;

    operator Container() const;

private:
    template <typename M, typename I, typename C> friend class MaskedRangeIterator;

    const MaskedRangeIteratorBase<Iterator> & base_;
};

} // namespace util
} // namespace treeDAG

#include "maskedRangeProxy.hxx"

#endif // TREEDAG_UTIL_MASKEDRANGEPROXY_HPP
