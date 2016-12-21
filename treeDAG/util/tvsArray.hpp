#ifndef TREEDAG_TVSARRAY_HPP
#define TREEDAG_TVSARRAY_HPP

#include <vector>
#include <boost/array.hpp>

namespace treeDAG {

template <typename T, int SIZE = -1>
class TVSArray;

template <typename T>
class TVSArray<T, -1> : public std::vector<T>
{
public:
    typedef std::vector<T> BASE;

    explicit TVSArray(std::size_t size = 0, const T & val = T());
};

template <typename T, int SIZE>
class TVSArray : public boost::array<T, SIZE>
{
public:
    typedef boost::array<T, SIZE> BASE;

    explicit TVSArray(std::size_t size = SIZE);
    explicit TVSArray(std::size_t size, const T & val);
};

template <typename T, int SIZE1, int SIZE2>
bool operator==(const TVSArray<T, SIZE1> & lhs, const TVSArray<T, SIZE2> & rhs);


} // namespace treeDAG

#include "tvsArray.hxx"

#endif // TREEDAG_TVSARRAY_HPP
