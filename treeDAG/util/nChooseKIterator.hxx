#ifndef TREEDAG_UTIL_NCHOOSEKITERATOR_HXX
#define TREEDAG_UTIL_NCHOOSEKITERATOR_HXX

#include "nChooseKIterator.hpp"

namespace treeDAG {
namespace util {

#define TDEF template <typename Iterator>
#define CDEF NChooseKIterator<Iterator>

TDEF
CDEF::NChooseKIterator()
    : Base(Iterator(), 1, true)
{
}

TDEF
CDEF::NChooseKIterator(Iterator first, Iterator last, std::size_t k)
    : Base(first, std::distance(first, last) + 1, false)
{
    // set the second-to-last k bits to true
    const std::size_t sz = Base::mask_.size() - 1;
    std::fill(Base::mask_.begin() + sz - k, Base::mask_.begin() + sz, true);
}

TDEF
CDEF::NChooseKIterator(Iterator first, std::size_t n, std::size_t k)
    : Base(first, n+1, false)
{
    std::fill(Base::mask_.begin() + n - k, Base::mask_.begin() + n, true);
}

#undef CDEF
#undef TDEF

} // namespace util
} // namespace treeDAG


#endif // TREEDAG_UTIL_NCHOOSEKITERATOR_HXX
