#ifndef TREEDAG_DECOMPOSER_HXX
#define TREEDAG_DECOMPOSER_HXX

#include "decomposer.hpp"
#include "util/kOrderedPermutateIterator.hpp"
#include "util/combination.hpp"
#include <iostream>


namespace treeDAG {

#define TDEF template <int SIZE, typename VertexIndexType>
#define CDEF Decomposer<SIZE, VertexIndexType>

TDEF
CDEF::Decomposer(const Graph * graph, std::size_t k)
    : cache_(k, graph),
      k_(k),
      graph_(graph)
{
}

TDEF
CDEF::Decomposer()
    : k_(0),
      graph_(0)
{
}


TDEF
void  CDEF::initialize()
{
    cache_.initialize();
}

TDEF
template <typename VertexIterator>
std::size_t CDEF::process(VertexIterator firstRoot, VertexIterator lastRoot)
{
    // store the roots
    roots_.assign(firstRoot, lastRoot);
    std::sort(roots_.begin(), roots_.end());

    // create the root requirement
    TreeDecompositionSubgraph root(boost::num_vertices(*graph_));
    for(std::size_t i = 0; i < root.size(); ++i)
        root[i] = TreeDecompositionSubgraph::ProjectedAwayVertex;
    for(; firstRoot != lastRoot; ++firstRoot)
        root[*firstRoot] = TreeDecompositionSubgraph::ActiveVertex;

    // add to the list
    todo_.push(root);

    // and process the complete todo set
    while(!todo_.empty())
    {
        // extract the top element
        TreeDecompositionSubgraph cur = todo_.top();
        todo_.pop();

        // already handled?
        if(!processed_.insert(cur).second)
            continue;

        // add to the dag
        dag_.addNode(cur);

        // process it
        processSubgraph(cur);
    }

    return processed_.size();
}

TDEF
void CDEF::processSubgraph(const TreeDecompositionSubgraph & subgraph)
{
    // start by extracting the active vertices
    std::vector<VertexIndexType> activeVertices, projectedVertices;
    for(std::size_t i = 0; i < subgraph.size(); ++i)
        if(subgraph[i] == TreeDecompositionSubgraph::ActiveVertex)
            activeVertices.push_back(i);
        else if(subgraph[i] == TreeDecompositionSubgraph::ProjectedAwayVertex)
            projectedVertices.push_back(i);

    // limits for the active vertices
    assert(activeVertices.size() > 0 && activeVertices.size() <= k_);

    // can we consider this as a leaf node?
    if(activeVertices.size() + projectedVertices.size() <= k_+1)
    {
        processLeafNode(subgraph, activeVertices, projectedVertices);
        return;
    }

    // how many should we add?
    std::size_t toAdd = k_ + 1 - activeVertices.size();

    // and now loop over all possible combinations we can add
    typedef util::KOrderedPermutateIterator<typename VertexVector::iterator> Iter;
    std::pair<Iter, Iter> p = util::make_k_ordered_permutate_range(projectedVertices.begin(), projectedVertices.end(), toAdd);

    for(std::pair<Iter, Iter> p = util::make_k_ordered_permutate_range(projectedVertices.begin(), projectedVertices.end(), toAdd); p.first != p.second; ++p.first)
        // and process it
        processClique(subgraph, activeVertices, *p.first);
}

TDEF
void CDEF::processClique(const TreeDecompositionSubgraph & subgraph, const VertexVector & oldActives, const VertexVector & newActives)
{
    typedef util::CombinationGenerator<typename VertexVector::const_iterator> CombGenerator;

    std::list<VertexVector> total;

    // create the clique
    VertexVector clique = oldActives;
    clique.insert(clique.end(), newActives.begin(), newActives.end());
    std::sort(clique.begin(), clique.end());

    // loop over all possible separators (having at least an element of newActives)

    CombGenerator newGenerator(newActives.begin(), newActives.end());
    do
    {
        // do we have at least one element
        VertexVector sep;
        newGenerator.current(std::back_inserter(sep));

        // we need one new active vertex
        if(sep.empty())
            continue;

        // okay, now loop over all old vertices
        CombGenerator oldGenerator(oldActives.begin(), oldActives.end());

        do
        {
            // get the total separator
            VertexVector possibleSeparator = sep;
            oldGenerator.current(std::back_inserter(possibleSeparator));

            // sorted
            std::sort(possibleSeparator.begin(), possibleSeparator.end());

            // is it a separator?
            const ComponentList * componentList = cache_.findSeparator(possibleSeparator);
            if(componentList == 0)
                continue;

            // add the separator
            total.push_back(possibleSeparator);

            // okay, now process the separator
            processSeparator(subgraph, possibleSeparator, *componentList);
        }
        while(oldGenerator.next());
    }
    while(newGenerator.next());

    if(!total.empty())
        // and add to the dag
        dag_.addClique(subgraph, clique, total.begin(), total.end());
}

TDEF
bool CDEF::hasIntersection(const VertexVector & lhs, const VertexVector & rhs) const
{
    typename VertexVector::const_iterator lit = lhs.begin(), rit = rhs.begin();
    while(lit != lhs.end() && rit != rhs.end())
    {
        if(*lit == *rit)
            return true;
        if(*lit < *rit)
            ++lit;
        else
            ++rit;
    }
    return false;
}

TDEF
void CDEF::processSeparator(const TreeDecompositionSubgraph & subgraph, const VertexVector & separatorVertices, const ComponentList & components)
{
    // already processed?
    if(!processedSeparators_.insert(separatorVertices).second)
        return;

    std::list<TreeDecompositionSubgraph> result;

    // let's first find the non-separator root vertices
    VertexVector nonSeparatorRoots;
    std::set_difference(roots_.begin(), roots_.end(), separatorVertices.begin(), separatorVertices.end(), std::inserter(nonSeparatorRoots, nonSeparatorRoots.end()));

    // we add all the components which do not intersect with the nonSeparatorRoots
    for(typename ComponentList::const_iterator it = components.begin(); it != components.end(); ++it)
    {
        // does it have an non-empty intersection
        if(hasIntersection(*it, nonSeparatorRoots))
            continue;

        // generate the request
        TreeDecompositionSubgraph request = constructComponent(subgraph, separatorVertices, *it);
        if(request.empty())
            continue;

        result.push_back(request);

        // already seen this request?
        if(processed_.find(request) == processed_.end())
            todo_.push(request);
    }

    // and create the node
    dag_.addSeparatorChildren(separatorVertices, result.begin(), result.end());
}

TDEF
TreeDecompositionSubgraph
CDEF::constructComponent(const TreeDecompositionSubgraph & subgraph, const VertexVector & separatorVertices, const VertexVector & component) const
{
    // check whether this is a good component
    TreeDecompositionSubgraph result(subgraph.size());

    // set all the projected away vertices
    for(typename VertexVector::const_iterator it = component.begin(); it != component.end(); ++it)
        if(subgraph[*it] != TreeDecompositionSubgraph::UnseenVertex)
            result[*it] = subgraph[*it];

    // set the separator vertices to active
    for(typename VertexVector::const_iterator it = separatorVertices.begin(); it != separatorVertices.end(); ++it)
        result[*it] = TreeDecompositionSubgraph::ActiveVertex;

    // okay, now fill in the separator vertices
    return result;
}



TDEF
typename CDEF::VertexVector
CDEF::extractAllBut(const VertexVector & source, std::size_t elementToForget) const
{
    // reserve the result
    VertexVector result(source.size() - 1);

    // and fill it in
    for(std::size_t i = 0; i < elementToForget; ++i)
        result[i] = source[i];
    for(std::size_t i = elementToForget+1; i < source.size(); ++i)
        result[i-1] = source[i];

    return result;
}

TDEF
void CDEF::processLeafNode(const TreeDecompositionSubgraph & subgraph, const VertexVector & activeVertices, const VertexVector & projectedVertices)
{
}

TDEF
void CDEF::writeDot(std::ostream & stream) const
{
    dag_.write_dot(stream);
}

} // namespace treeDAG

#endif // TREEDAG_DECOMPOSER_HXX
