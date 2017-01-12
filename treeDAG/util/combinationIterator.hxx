#include "combinationIterator.hpp"

namespace treeDAG {
namespace util {


#define TDEF template <typename Iterator>
#define CDEF CombinationIterator<Iterator>


TDEF
CDEF::CombinationIterator()
    : Base(Iterator(), 1, true)
{
}

TDEF
CDEF::CombinationIterator(Iterator first, Iterator last)
    : Base(first, std::distance(first, last) + 1, false)
{

}


#undef CDEF
#undef TDEF


} // namespace util
} // namespace treeDAG
