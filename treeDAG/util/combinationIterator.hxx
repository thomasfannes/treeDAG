#include "combinationIterator.hpp"

namespace treeDAG {
namespace util {

#define TDEF template <typename Iterator>
#define CDEF CombinationValueIterator<Iterator>

TDEF
CDEF::CombinationValueIterator(Iterator first, const std::vector<bool> * mask)
    : cur_(first),
      pos_(0),
      mask_(mask)
{
    gotoFirstValid();
}

TDEF
CDEF::CombinationValueIterator()
    : cur_(0),
      pos_(0),
      mask_(0)
{
}

TDEF
void CombinationValueIterator<Iterator>::increment()
{
    ++pos_;
    ++cur_;
    gotoFirstValid();
}

TDEF
bool CDEF::equal(const CombinationValueIterator<Iterator> & other) const
{
    if(atEnd() && other.atEnd())
        return true;

    if(atEnd() || other.atEnd())
        return false;

    return mask_ == other.mask_ && pos_ == other.pos_;
}

TDEF
typename std::iterator_traits<Iterator>::reference CDEF::dereference() const
{
    return *cur_;
}

TDEF
bool CDEF::atEnd() const
{
    return mask_ == 0 || pos_ == mask_->size() - 1;
}

TDEF
void CDEF::gotoFirstValid()
{
    while(pos_ < mask_->size() - 1 && !mask_->operator [](pos_))
    {
        ++pos_;
        ++cur_;
    }
}

#undef CDEF
#define CDEF CombinationValueProxy<Iterator>

TDEF
CDEF::CombinationValueProxy(const CombinationIterator<Iterator> * base)
    : base_(base)
{
}

TDEF
std::pair<typename CDEF::iterator, typename CDEF::iterator> CDEF::range() const
{
    return std::make_pair(begin(), end());
}
TDEF
typename CDEF::iterator CDEF::begin() const
{
    return iterator(base_->first_, &base_->mask_);
}

TDEF
typename CDEF::iterator CDEF::end() const
{
    return iterator();
}

#undef CDEF
#define CDEF CombinationIterator<Iterator>

TDEF
CDEF::CombinationIterator()
    : first_(Iterator()),
      mask_(1, true),
      proxy_(0)
{

}

TDEF
CDEF::CombinationIterator(const std::pair<Iterator, Iterator> & range)
    : first_(range.first),
      mask_(std::distance(range.first, range.second) + 1, false),
      proxy_(this)
{
}


TDEF
CDEF::CombinationIterator(Iterator first, Iterator last)
    : first_(first),
      mask_(std::distance(first, last) + 1, false),
      proxy_(this)
{

}

TDEF
CDEF::CombinationIterator(const CombinationIterator & other)
    : first_(other.first_),
      mask_(other.mask_),
      proxy_(this)
{
}

TDEF
CDEF & CDEF::operator=(const CombinationIterator & other)
{
    first_ = other.first_;
    mask_ = other.mask_;

    return *this;
}

TDEF
void CDEF::increment()
{
    // find the first false value
    std::size_t flipPos = 0;
    while(mask_[flipPos] && flipPos < mask_.size())
        ++flipPos;

    // can we flip it to true?
    if(flipPos < mask_.size())
        mask_[flipPos] = true;

    // set the rest to false
    std::fill(mask_.begin(), mask_.begin() + flipPos, false);
}

TDEF
bool CDEF::equal(const CombinationIterator<Iterator> & other) const
{
    if(atEnd() && other.atEnd())
        return true;

    if(atEnd() || other.atEnd())
        return false;

    return first_ == other.first_ && mask_ == other.mask_;
}

TDEF
const CombinationValueProxy<Iterator> & CDEF::dereference() const
{
    return proxy_;
}

TDEF
bool CDEF::atEnd() const
{
    return mask_.back();
}



#undef CDEF
#undef TDEF





} // namespace util
} // namespace treeDAG
