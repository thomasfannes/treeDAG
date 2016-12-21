#ifndef TREEDAG_TVSARRAY_HXX
#define TREEDAG_TVSARRAY_HXX

#include "tvsArray.hpp"

namespace treeDAG {

#define TDEF template <typename T, int SIZE>
#define CDEF TVSArray<T, SIZE>

TDEF
CDEF::TVSArray(std::size_t size)
{
    assert(size == SIZE);
}

TDEF
CDEF::TVSArray(std::size_t size, const T & val)
{
    assert(size == SIZE);
    this->assign(val);
}


#undef TDEF
#undef CDEF
#define TDEF template <typename T>
#define CDEF TVSArray<T, -1>

TDEF
CDEF::TVSArray(std::size_t size, const T & val)
    : std::vector<T>(size, val)
{
}


#undef TDEF
#undef CDEF

template <typename T, int SIZE1, int SIZE2>
bool operator==(const TVSArray<T, SIZE1> & lhs, const TVSArray<T, SIZE2> & rhs)
{
    // positive sizes? then they should be equal
    if(SIZE1 > 0 && SIZE2 > 0 && SIZE1 != SIZE2)
        return false;

    // do they have the same size?
    if(lhs.size() != rhs.size())
        return false;

    // elementwise check
    for(std::size_t i = 0; i < lhs.size(); ++i)
        if(lhs[i] != rhs[i])
            return false;

    return true;
}

} // namespace treeDAG

#endif // TREEDAG_TVSARRAY_HXX
