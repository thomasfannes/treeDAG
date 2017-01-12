#include "maskedRangeIterator.hpp"

namespace treeDAG {
namespace util {

#define TDEF template <typename MaskProcessor, typename Iterator, typename Container>
#define CDEF MaskedRangeIterator<MaskProcessor, Iterator, Container>


TDEF
CDEF::MaskedRangeIterator()
    : proxy_(*this)
{
}

TDEF
CDEF::MaskedRangeIterator(Iterator first, std::size_t maskSize, bool initialMaskValue)
    : Base(first, maskSize, initialMaskValue),
      proxy_(*this)
{
}

TDEF
CDEF::MaskedRangeIterator(const CDEF & rhs)
    : Base(rhs),
      proxy_(*this)
{
}

TDEF
CDEF & CDEF::operator=(const CDEF & rhs)
{
    static_cast<Base &>(*this) = rhs;
    return *this;
}

TDEF
bool CDEF::equal(const CDEF & rhs) const
{
    bool mAtEnd = MaskProcessor::atEnd(Base::mask_);
    bool rAtEnd = rhs.atEnd(rhs.mask_);

    if(mAtEnd && rAtEnd)
        return true;

    if(mAtEnd || rAtEnd)
        return false;

    return Base::first_ == rhs.first_ && Base::mask_ == rhs.mask_;
}


TDEF
void CDEF::increment()
{
    MaskProcessor::increment(Base::mask_);
}


TDEF
const MaskedRangeProxy<Iterator, Container> & CDEF::dereference() const
{
    return proxy_;
}


#undef TDEF
#undef CDEF


} // namespace util
} // namespace treeDAG
