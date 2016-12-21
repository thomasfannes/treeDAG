/*******************************************************************
* Copyright (c) 2010-2013 MiGraNT - DTAI.cs.kuleuven.be
* License details in the root directory of this distribution in
* the LICENSE file
********************************************************************/

#ifndef TREEDAG_TREEDECOMPOSITIONSUBGRAPH_HPP
#define TREEDAG_TREEDECOMPOSITIONSUBGRAPH_HPP

#include <vector>
#include <ostream>


namespace treeDAG {

class TreeDecompositionSubgraph : private std::vector<int>
{
    typedef std::vector<int> Base;

public:
    enum ElementType
    {
        UnseenVertex = 0,
        ProjectedAwayVertex = 255,
        ActiveVertex = 1
    };

    explicit TreeDecompositionSubgraph(std::size_t patternSize = 0);

    std::size_t size() const;
    bool empty() const;
    int operator[](std::size_t pos) const;
    int & operator[](std::size_t pos);

    template <typename It> void assign(It first, It second, ElementType value);

    void reset();

    std::size_t count(ElementType type) const;

    bool operator==(const TreeDecompositionSubgraph & rhs) const;
    bool operator!=(const TreeDecompositionSubgraph & rhs) const;
    bool operator<(const TreeDecompositionSubgraph & rhs) const;
    bool operator<=(const TreeDecompositionSubgraph & rhs) const;
    bool operator>(const TreeDecompositionSubgraph & rhs) const;
    bool operator>=(const TreeDecompositionSubgraph & rhs) const;

    bool isValid(std::size_t patternSize) const;

    TreeDecompositionSubgraph & operator+=(const TreeDecompositionSubgraph & rhs);
    TreeDecompositionSubgraph operator+(const TreeDecompositionSubgraph & rhs) const;

    friend std::size_t hash_value(const TreeDecompositionSubgraph & subgraph);
    friend void swap(TreeDecompositionSubgraph & lhs, TreeDecompositionSubgraph & rhs);
};

bool partial_order_compare(const TreeDecompositionSubgraph & lhs, const TreeDecompositionSubgraph & rhs);
std::ostream & operator<<(std::ostream & stream, const TreeDecompositionSubgraph & subgraph);

std::size_t hash_value(const TreeDecompositionSubgraph & subgraph);


template <typename It>
void TreeDecompositionSubgraph::assign(It first, It last, ElementType value)
{
    for(; first != last; ++first)
        operator [](*first) = value;
}

} // namespace treeDAG

#endif // TREEDAG_TREEDECOMPOSITIONSUBGRAPH_HPP
