#ifndef TREEDAG_UTIL_MASKEDRANGEITERATOR_HPP
#define TREEDAG_UTIL_MASKEDRANGEITERATOR_HPP

#include <boost/iterator/iterator_facade.hpp>
#include <vector>
#include "maskedRangeProxy.hpp"

namespace treeDAG {
namespace util {

template <typename Iterator, typename Container> class MaskedRangeProxy;

template <typename Iterator>
struct MaskedRangeIteratorBase
{
    MaskedRangeIteratorBase() : first_(Iterator()) {}
    MaskedRangeIteratorBase(Iterator first, std::size_t maskSize, bool maskValue)
        : mask_(maskSize, maskValue),
          first_(first)
    {
    }

    std::vector<bool> mask_;
    Iterator first_;
};

template <
        typename MaskProcessor,
        typename Iterator,
        typename Container = std::vector<typename std::iterator_traits<Iterator>::value_type >
        >
class MaskedRangeIterator
        :
        public MaskedRangeIteratorBase<Iterator>,
        public MaskProcessor,
        public boost::iterator_facade
            <
                MaskedRangeIterator<MaskProcessor, Iterator, Container>,
                MaskedRangeProxy<Iterator, Container>,
                boost::forward_traversal_tag,
                const MaskedRangeProxy<Iterator, Container> &
            >
{
    typedef MaskedRangeIteratorBase<Iterator> Base;

public:
    MaskedRangeIterator();
    MaskedRangeIterator(Iterator first, std::size_t maskSize, bool initialMaskValue);
    MaskedRangeIterator(const MaskedRangeIterator<MaskProcessor, Iterator, Container> & rhs);

    MaskedRangeIterator<MaskProcessor, Iterator, Container> & operator=(const MaskedRangeIterator<MaskProcessor, Iterator, Container> & rhs);

private:
    friend class boost::iterator_core_access;

    bool equal(const MaskedRangeIterator<MaskProcessor, Iterator, Container> & rhs) const;
    void increment();

    const MaskedRangeProxy<Iterator, Container> & dereference() const;

    MaskedRangeProxy<Iterator, Container> proxy_;
};


} // namespace util
} // namespace treeDAG

#include "maskedRangeIterator.hxx"

#endif // TREEDAG_UTIL_MASKEDRANGEITERATOR_HPP
