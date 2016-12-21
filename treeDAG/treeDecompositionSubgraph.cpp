/*******************************************************************
* Copyright (c) 2010-2013 MiGraNT - DTAI.cs.kuleuven.be
* License details in the root directory of this distribution in
* the LICENSE file
********************************************************************/

#include "treeDecompositionSubgraph.hpp"
#include <algorithm>
#include <stdexcept>
#include <boost/functional/hash.hpp>

namespace treeDAG {

TreeDecompositionSubgraph::TreeDecompositionSubgraph(std::size_t patternSize)
    : Base(patternSize, UnseenVertex)
{
}

bool TreeDecompositionSubgraph::isValid(std::size_t patternSize) const
{
    if(this->size() != patternSize)
        return false;

    for(std::size_t i = 0; i < patternSize; ++i)
    {
        int curValue = at(i);
        if(curValue != ProjectedAwayVertex && curValue != ActiveVertex && curValue != UnseenVertex)
            return false;
    }

    return true;

}

std::size_t hash_value(const TreeDecompositionSubgraph & subgraph)
{
    if(subgraph.size() == 0)
        return 0;

    return boost::hash_range(subgraph.begin(), subgraph.end());
}

bool TreeDecompositionSubgraph::operator==(const TreeDecompositionSubgraph & rhs) const
{
    return static_cast<const Base &>(*this) == static_cast<const Base &>(rhs);
}

bool TreeDecompositionSubgraph::operator!=(const TreeDecompositionSubgraph & rhs) const
{
    return !this->operator ==(rhs);
}
std::size_t TreeDecompositionSubgraph::size() const
{
    return Base::size();
}

bool TreeDecompositionSubgraph::empty() const
{
    return Base::empty();
}

int TreeDecompositionSubgraph::operator[](std::size_t pos) const
{
    return Base::operator[](pos);
}

std::size_t TreeDecompositionSubgraph::count(ElementType type) const
{
    return std::count(begin(), end(), type);
}

int & TreeDecompositionSubgraph::operator[](std::size_t pos)
{
    return Base::operator[](pos);
}

void TreeDecompositionSubgraph::reset()
{
    Base::assign(size(), UnseenVertex);
}

bool TreeDecompositionSubgraph::operator<(const TreeDecompositionSubgraph & rhs) const
{
    return static_cast<const Base & >(*this) < static_cast<const Base &>(rhs);
}
bool TreeDecompositionSubgraph::operator<=(const TreeDecompositionSubgraph & rhs) const
{
    return static_cast<const Base & >(*this) <= static_cast<const Base &>(rhs);
}
bool TreeDecompositionSubgraph::operator>(const TreeDecompositionSubgraph & rhs) const
{
    return static_cast<const Base & >(*this) > static_cast<const Base &>(rhs);
}
bool TreeDecompositionSubgraph::operator>=(const TreeDecompositionSubgraph & rhs) const
{
    return static_cast<const Base & >(*this) >= static_cast<const Base &>(rhs);
}

TreeDecompositionSubgraph & TreeDecompositionSubgraph::operator+=(const TreeDecompositionSubgraph & rhs)
{
    if(size() != rhs.size())
        throw std::logic_error("TreeDecompositionSubgraph: Unable to combine both tree decomposition subgraphs");

    TreeDecompositionSubgraph & me = *this;

    for(std::size_t i = 0; i < size(); ++i)
    {
        if(me[i] == TreeDecompositionSubgraph::ProjectedAwayVertex || rhs[i] == TreeDecompositionSubgraph::ProjectedAwayVertex)
        {
            if(me[i] != TreeDecompositionSubgraph::UnseenVertex || rhs[i] != TreeDecompositionSubgraph::UnseenVertex)
                throw std::logic_error("TreeDecompositionSubgraph: Unable to combine both tree decomposition subgraphs");

            me[i] = TreeDecompositionSubgraph::ProjectedAwayVertex;
        }
        else if(me[i] == TreeDecompositionSubgraph::ActiveVertex || rhs[i] == TreeDecompositionSubgraph::ActiveVertex)
        {
            me[i] = TreeDecompositionSubgraph::ActiveVertex;
        }
    }

    return me;
}

TreeDecompositionSubgraph TreeDecompositionSubgraph::operator+(const TreeDecompositionSubgraph & rhs) const
{
    if(size() != rhs.size())
        throw std::logic_error("TreeDecompositionSubgraph: Unable to combine both tree decomposition subgraphs");

    TreeDecompositionSubgraph result(*this);
    result += rhs;

    return result;
}


bool partial_order_compare(const TreeDecompositionSubgraph & lhs, const TreeDecompositionSubgraph & rhs)
{
    if(lhs.size() != rhs.size())
        return false;

    for(std::size_t i = 0; i < lhs.size(); ++i)
    {
        switch(lhs[i])
        {
        case TreeDecompositionSubgraph::UnseenVertex:
            break;

        case TreeDecompositionSubgraph::ProjectedAwayVertex:
            if(rhs[i] == TreeDecompositionSubgraph::UnseenVertex)
                return false;
            break;

        case TreeDecompositionSubgraph::ActiveVertex:
            if(rhs[i] != TreeDecompositionSubgraph::ActiveVertex)
                return false;
            break;
        }
    }

    return true;
}

std::ostream & operator<<(std::ostream & str, const TreeDecompositionSubgraph & subgraph)
{
    str << "{";
    unsigned int written = 0;
    for(unsigned int i = 0; i < subgraph.size(); ++i)
        if(subgraph[i] == TreeDecompositionSubgraph::ActiveVertex)
        {
            if(written++ != 0)
                str << ", ";
            str << "v" << i;
        }
    str << "}, *{";
    written = 0;
    for(unsigned int i = 0; i < subgraph.size(); ++i)
        if(subgraph[i] == TreeDecompositionSubgraph::ProjectedAwayVertex)
        {
            if(written++ != 0)
                str << ", ";
            str << "v" << i;
        }
    str << "}";

    return str;
}

void swap(TreeDecompositionSubgraph & lhs, TreeDecompositionSubgraph & rhs)
{
    using std::swap;
    swap(static_cast<TreeDecompositionSubgraph::Base &>(lhs), static_cast<TreeDecompositionSubgraph::Base &>(rhs));
}

} // namespace treeDAG

