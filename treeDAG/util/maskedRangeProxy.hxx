#ifndef TREEDAG_UTIL_MASKEDRANGEPROXY_HXX
#define TREEDAG_UTIL_MASKEDRANGEPROXY_HXX

#include "maskedRangeProxy.hpp"

namespace treeDAG {
namespace util {

#define TDEF template <typename Iterator, typename Container>
#define CDEF MaskedRangeProxy<Iterator, Container>

TDEF
CDEF::iterator::iterator(Iterator first, const std::vector<bool> * mask)
    : cur_(first),
      pos_(0),
      mask_(mask)
{
    gotoFirstValid();
}

TDEF
CDEF::iterator::iterator()
    : cur_(0),
      pos_(0),
      mask_(0)
{
}

TDEF
void CDEF::iterator::increment()
{
    ++pos_;
    ++cur_;
    gotoFirstValid();
}

TDEF
bool CDEF::iterator::equal(const iterator & other) const
{
    if(atEnd() && other.atEnd())
        return true;

    if(atEnd() || other.atEnd())
        return false;

    return mask_ == other.mask_ && pos_ == other.pos_;
}

TDEF
typename std::iterator_traits<Iterator>::reference CDEF::iterator::dereference() const
{
    return *cur_;
}

TDEF
bool CDEF::iterator::atEnd() const
{
    return mask_ == 0 || pos_ == mask_->size() - 1;
}

TDEF
void CDEF::iterator::gotoFirstValid()
{
    while(pos_ < mask_->size() - 1 && !mask_->operator [](pos_))
    {
        ++pos_;
        ++cur_;
    }
}


TDEF
CDEF::MaskedRangeProxy(const MaskedRangeIteratorBase<Iterator> & base)
    : base_(base)
{
}


TDEF
CDEF::MaskedRangeProxy(const CDEF & rhs)
{
}

TDEF
CDEF & CDEF::operator=(const CDEF & rhs)
{
}

TDEF
CDEF::operator Container() const
{
    return Container(begin(), end());
}

TDEF
std::pair<typename CDEF::iterator, typename CDEF::iterator> CDEF::range() const
{
    return std::make_pair(begin(), end());
}

TDEF
typename CDEF::iterator CDEF::begin() const
{
    return iterator(base_.first_, &base_.mask_);
}

TDEF
typename CDEF::iterator CDEF::end() const
{
    return iterator();
}


#undef TDEF
#undef CDEF

} // namespace util
} // namespace treeDAG

#endif // TREEDAG_UTIL_MASKEDRANGEPROXY_HXX
