#ifndef TREEDAG_UTIL_COMBINATION_HXX
#define TREEDAG_UTIL_COMBINATION_HXX

#include "combination.hpp"

namespace treeDAG {
namespace util {

#define TDEF template <typename Iterator>
#define CDEF CombinationGenerator<Iterator>

TDEF
CDEF::CombinationGenerator()
    : first_(Iterator()),
      last_(Iterator())
{
}

TDEF
CDEF::CombinationGenerator(Iterator first, Iterator last)
    : first_(first),
      last_(last),
      indices_(std::distance(first, last), false)
{
}

TDEF
void CDEF::reset()
{

}

TDEF
template <typename OutIterator>
void CDEF::current(OutIterator it) const
{
    Iterator cur = first_;
    for(std::size_t i = 0; i < indices_.size(); ++i, ++cur)
        if(indices_[i])
            (*it)++ = *cur;
}

TDEF
std::vector<typename std::iterator_traits<Iterator>::value_type>
CDEF::current() const
{
    std::vector<typename std::iterator_traits<Iterator>::value_type> vct;
    current(std::back_inserter(vct));

    return vct;
}

TDEF
bool CDEF::next()
{
    std::size_t firstFalse = indices_.size();

    // find the first false element
    for(std::size_t i = 0; i < indices_.size() && firstFalse == indices_.size(); ++i)
        if(indices_[i] == false)
            firstFalse = i;

    // swap everything just up to firstFalse
    for(std::size_t i = 0; i < firstFalse; ++i)
        indices_[i] = false;

    // found a final bit to toggle>
    if(firstFalse == indices_.size())
        return false;

    // and now update the indices
    indices_[firstFalse] = true;
    return true;
}

#undef TDEF
#undef CDEF

} // namespace util
} // namespace treeDAG

#endif // TREEDAG_UTIL_COMBINATION_HXX
