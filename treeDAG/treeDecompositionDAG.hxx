/*******************************************************************
* Copyright (c) 2010-2013 MiGraNT - DTAI.cs.kuleuven.be
* License details in the root directory of this distribution in
* the LICENSE file
********************************************************************/


#ifndef TREEDAG_TREEDECOMPOSITIONDAG_HXX
#define TREEDAG_TREEDECOMPOSITIONDAG_HXX

#include "treeDecompositionDAG.hpp"
#include "treeDecompositionDAGAndNode.hpp"
#include <stack>

namespace treeDAG {

template <typename Iterator>
TreeDecompositionSubgraph TreeDecompositionDAG::addList(Iterator firstVertex, Iterator lastVertex)
{
    std::size_t count = 0;
    TreeDecompositionSubgraph comp(patternSize_);

    for(; firstVertex != lastVertex; ++firstVertex, ++count)
    {
        if(*firstVertex >= patternSize_)
            throw errOutOfBounds();

        comp[*firstVertex] = TreeDecompositionSubgraph::ActiveVertex;
    }

    if(count == 0)
        throw errBadRange();

    // add the element
    return addUnaryAnd(TreeDecompositionSubgraph(patternSize()), comp, TreeDecompositionDAGLabel::NODE_ChildrenList);
}

template <typename Iterator>
TreeDecompositionSubgraph TreeDecompositionDAG::addProject(const TreeDecompositionSubgraph & childComponent, Iterator first, Iterator last)
{
    std::size_t count = 0;
    TreeDecompositionSubgraph comp(childComponent);

    for(; first != last; ++first, ++count)
    {
        std::size_t pos = *first;

        if(pos >= patternSize_)
            throw errOutOfBounds();

        if(comp[pos] != TreeDecompositionSubgraph::ActiveVertex)
            throw errNotActive();

        comp[pos] = TreeDecompositionSubgraph::ProjectedAwayVertex;
    }

    if(count == 0)
        throw errBadRange();

    return addUnaryAnd(childComponent, comp, TreeDecompositionDAGLabel::NODE_ChildrenProject);
}


template <typename Iterator>
TreeDecompositionSubgraph TreeDecompositionDAG::addExtend(const TreeDecompositionSubgraph & childComponent, Iterator firstVertexToExtend, Iterator lastVertexToExtend)
{
    std::vector<std::size_t> vct(firstVertexToExtend, lastVertexToExtend);
    std::sort(vct.begin(), vct.end());

    if(vct.empty())
        throw errBadRange();

    TreeDecompositionSubgraph tgt(childComponent);
    do
    {
        for(std::size_t i = 0; i < vct.size(); ++i)
            tgt = addExtend(tgt, vct[i]);
    }
    while(std::next_permutation(vct.begin(), vct.end()));

    return tgt;
}

template <typename Iterator>
TreeDecompositionSubgraph TreeDecompositionDAG::addJoin(Iterator firstComponent, Iterator lastComponent)
{
    std::vector<TreeDecompositionSubgraph> vct(firstComponent, lastComponent);

    if(vct.size() == 0)
        throw errBadRange();

    TreeDecompositionSubgraph result(patternSize());
    for(std::size_t i = 0; i < vct.size(); ++i)
        result += vct[i];

    addNaryAnd(vct, result, TreeDecompositionDAGLabel::NODE_ChildrenJoin);

    return result;
}

template <typename Iterator>
TreeDecompositionDAG::InsertResult TreeDecompositionDAG::checkForInsertion(const TreeDecompositionSubgraph & component, Iterator firstRequiredVertex, Iterator lastRequiredVertex) const
{
    bool thisPossible = true;

    for(; firstRequiredVertex != lastRequiredVertex; ++firstRequiredVertex)
        if(component[*firstRequiredVertex] == TreeDecompositionSubgraph::UnseenVertex)
            return NotPossible;
        else if(component[*firstRequiredVertex] == TreeDecompositionSubgraph::ProjectedAwayVertex)
            thisPossible = false;

    return (thisPossible ? CanInsert : MaybeIntoChildren);
}

template <typename Iterator>
void TreeDecompositionDAG::removeNodes(Iterator firstComponentIterator, Iterator lastComponentIterator)
{
    typedef boost::graph_traits<Graph>::in_edge_iterator iet;
    typedef boost::graph_traits<Graph>::out_edge_iterator oei;

    std::set<Node> andNodes;

    for(Iterator it = firstComponentIterator; it != lastComponentIterator; ++it)
    {
        const TreeDecompositionSubgraph & component = *it;
        Node n = findExistingNode(component);

        if(n == invalidDescriptor())
            throw errBadRange();

        // add the and nodes to clear
        for(std::pair<iet, iet> p = boost::in_edges(n, structure_); p.first != p.second; ++p.first)
            andNodes.insert(boost::source(*p.first, structure_));
        for(std::pair<oei, oei> p = boost::out_edges(n, structure_); p.first != p.second; ++p.first)
            andNodes.insert(boost::target(*p.first, structure_));
        andNodes.insert(n);

        // cleanup the map
        map_.erase(component);
    }

    // clear the nodes
    for(std::set<Node>::const_iterator it = andNodes.begin(); it != andNodes.end(); ++it)
    {
        boost::clear_vertex(*it, structure_);
        boost::remove_vertex(*it, structure_);
    }
}


template <typename Iterator> void TreeDecompositionDAG::visitComponents(const TreeDecompositionSubgraph & rootComponent, Iterator outputIterator) const
{
    Node n = findExistingNode(rootComponent);
    if(n == invalidDescriptor())
        return;

    std::set<Node> visitedNodes;
    std::stack<Node> todo;
    todo.push(n);

    while(!todo.empty())
    {
        Node cur = todo.top();
        todo.pop();

        if(!visitedNodes.insert(cur).second)
            continue;

        const TreeDecompositionDAGLabel & lbl = structure_[cur];

        if(lbl.type != TreeDecompositionDAGLabel::NODE_ChildrenOR)
            continue;

        *outputIterator++ = lbl.vertices;
    }
}


} // treeDAG namespace

#endif // TREEDAG_TREEDECOMPOSITIONDAG_HXX

